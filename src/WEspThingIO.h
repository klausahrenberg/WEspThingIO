#ifndef W_ESP_THING_IO_H
#define W_ESP_THING_IO_H

#include "WDevice.h"
#include "WEspThingIOHtml.h"
#include "WHtmlPages.h"
#include "WPage.h"
#include "WProps.h"
#include "WRelay.h"
#include "WSwitch.h"
#include "hw/WHtu21D.h"
#include "hw/W2812.h"

#define DEVICE_ID "switch"
#define MAX_GPIOS 12
#define MAX_PROP_BYTES 4
#define GPIO_TYPE_LED 0
#define GPIO_TYPE_RELAY 1
#define GPIO_TYPE_BUTTON 2
#define GPIO_TYPE_SWITCH 3
#define GPIO_TYPE_MODE 4
#define GPIO_TYPE_RGB_LED 5
#define GPIO_TYPE_MERGE 6
#define GPIO_TYPE_HTU21D 7
#define GPIO_TYPE_UNKNOWN 0xFF

#define BYTE_TYPE 0
#define BYTE_GPIO 1
#define BYTE_CONFIG 2
#define BYTE_SCL 3
#define BYTE_NO_OF_LEDS BYTE_SCL
#define BYTE_NO_GPIO 0xFF
#define NO_INDEX BYTE_NO_GPIO

#define FIRST_NAME 0
#define FIRST_TITLE 1
#define SECOND_NAME 2
#define SECOND_TITLE 3

#define BIT_CONFIG_PROPERTY_GROUPED 0
#define BIT_CONFIG_PROPERTY_MQTT 1
#define BIT_CONFIG_PROPERTY_WEBTHING 2
#define BIT_CONFIG_LED_INVERTED 3
#define BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE  BIT_CONFIG_LED_INVERTED
#define BIT_CONFIG_NO_OF_LEDS                    BIT_CONFIG_LED_INVERTED
#define BIT_CONFIG_LED_LINKSTATE 4
#define BIT_CONFIG_SWITCH_AP_LONGPRESS BIT_CONFIG_LED_LINKSTATE
#define BIT_CONFIG_SWITCH_DIRECTLY 5

#define CAPTION_EDIT "Edit"
#define CAPTION_REMOVE "Remove"
#define CAPTION_OK "Ok"
#define CAPTION_CANCEL "Cancel"
#define HTTP_USE_TEMPLATE "usetemplate"
#define HTTP_ADD_LED "addled"
#define HTTP_ADD_RELAY "addrelay"
#define HTTP_ADD_RGB_LED "addrgbled"
#define HTTP_ADD_HTU21D "addht21d"
#define HTTP_ADD_BUTTON "addbutton"
#define HTTP_ADD_SWITCH "addswitch"
#define HTTP_ADD_MODE "addmode"
#define HTTP_ADD_MERGE "addmerge"
#define HTTP_REMOVE_GPIO "removegpio"
#define GPIO_DEFAULT_ID "^g_"

const byte* DEFAULT_PROP_ARRAY = (const byte[]){0, 0, 0, 0};
const static char HTTP_BUTTON_VALUE[] PROGMEM = R"=====(
	<div>
  	<form action='/%s' method='POST'>
    	<button class='%s' name='gpio' value='%s'>%s</button>
    </form>
  </div>
)=====";

class WMergedOutput: public WOutput {
 public:
  WMergedOutput(WProperty* merged)
    : WOutput(NO_PIN) {
    _merged = merged;  
  }

  void onChanged() {
    WOutput::onChanged();    
    _merged->setBoolean(this->isOn());
  };  

 private:
  WProperty* _merged;
};

class WEspThingIO : public WDevice {
 public:
  WEspThingIO(WNetwork* network)
      : WDevice(network, DEVICE_ID, DEVICE_ID, DEVICE_TYPE_ON_OFF_SWITCH,
                DEVICE_TYPE_LIGHT) {
    _editingItem = nullptr;
    _editingIndex = NO_INDEX;
    _numberOfGPIOs = network->settings()->setByte("numberOfGPIOs", 0, MAX_GPIOS);
    _numberOfGPIOs->setVisibility(NONE);
    this->addProperty(_numberOfGPIOs);
    _numberOfSettings = network->settings()->size();
    byte nog = _numberOfGPIOs->getByte();
    _numberOfGPIOs->setByte(0);
    for (byte i = 0; i < nog; i++) {
      _addGpioConfig(GPIO_TYPE_UNKNOWN);
      _numberOfGPIOs->setByte(i + 1);
    }    
    // Configure Device
    _configureDevice();    
    this->setVisibility(
        ALL);  // this->showAsWebthingDevice->getBoolean() ? ALL : MQTT);
    // HtmlPages
    WPage* configPage = new WPage(this->id(), "Configure device");
    configPage->setPrintPage(std::bind(&WEspThingIO::_printConfigPage, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2));
    configPage->setSubmittedPage(std::bind(&WEspThingIO::_saveConfigPage, this,
                                           std::placeholders::_1,
                                           std::placeholders::_2));
    network->addCustomPage(configPage);
    // Add LED
    WPage* utPage = new WPage(HTTP_USE_TEMPLATE, "Use a template");
    utPage->setShowInMainMenu(false);
    utPage->setPrintPage(std::bind(&WEspThingIO::_handleHttpUseTemplate, this,
                                   std::placeholders::_1,
                                   std::placeholders::_2));
    utPage->setSubmittedPage(
        std::bind(&WEspThingIO::_handleHttpSubmitUseTemplate, this,
                  std::placeholders::_1, std::placeholders::_2));
    utPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(utPage);
    // Add LED
    WPage* ledPage = new WPage(HTTP_ADD_LED, "Add/edit LED");
    ledPage->setShowInMainMenu(false);
    ledPage->setPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioLed, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
    ledPage->setSubmittedPage(
        std::bind(&WEspThingIO::_handleHttpSubmitAddGpioLed, this,
                  std::placeholders::_1, std::placeholders::_2));
    ledPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(ledPage);
    // Add Relay
    WPage* relayPage = new WPage(HTTP_ADD_RELAY, "Add/edit Relay");
    relayPage->setShowInMainMenu(false);
    relayPage->setPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioRelay,
                                      this, std::placeholders::_1,
                                      std::placeholders::_2));
    relayPage->setSubmittedPage(
        std::bind(&WEspThingIO::_handleHttpSubmitAddGpioRelay, this,
                  std::placeholders::_1, std::placeholders::_2));
    relayPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(relayPage);
    // Add RGB LED
    WPage* rgbledPage = new WPage(HTTP_ADD_RGB_LED, "Add/edit RGB LED");
    rgbledPage->setShowInMainMenu(false);
    rgbledPage->setPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioRgbLed,
                                       this, std::placeholders::_1,
                                       std::placeholders::_2));
    rgbledPage->setSubmittedPage(
        std::bind(&WEspThingIO::_handleHttpSubmitAddGpioRgbLed, this,
                  std::placeholders::_1, std::placeholders::_2));
    rgbledPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(rgbledPage);
    // Add Switch
    WPage* switchPage = new WPage(HTTP_ADD_SWITCH, "Add/edit Switch");
    switchPage->setShowInMainMenu(false);
    switchPage->setPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioSwitch,
                                       this, std::placeholders::_1,
                                       std::placeholders::_2));
    switchPage->setSubmittedPage(
        std::bind(&WEspThingIO::_handleHttpSubmitAddGpioSwitch, this,
                  std::placeholders::_1, std::placeholders::_2));
    switchPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(switchPage);
    // Add Button
    WPage* buttonPage = new WPage(HTTP_ADD_BUTTON, "Add/edit Button");
    buttonPage->setShowInMainMenu(false);
    buttonPage->setPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioButton,
                                       this, std::placeholders::_1,
                                       std::placeholders::_2));
    buttonPage->setSubmittedPage(
        std::bind(&WEspThingIO::_handleHttpSubmitAddGpioButton, this,
                  std::placeholders::_1, std::placeholders::_2));
    buttonPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(buttonPage);
    // Add HTU21
    WPage* htuPage = new WPage(HTTP_ADD_HTU21D, "Add/edit HTU21D");
    htuPage->setShowInMainMenu(false);
    htuPage->setPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioHtu21, this, std::placeholders::_1, std::placeholders::_2));
    htuPage->setSubmittedPage(std::bind(&WEspThingIO::_handleHttpSubmitAddGpioHtu21, this, std::placeholders::_1, std::placeholders::_2));
    htuPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(htuPage);
    // Grouped - Mode
    WPage* groupedPage = new WPage(HTTP_ADD_MODE, "Add/edit Mode Property");
    groupedPage->setShowInMainMenu(false);
    groupedPage->setPrintPage(std::bind(&WEspThingIO::_handleHttpAddModeProperty,
                                        this, std::placeholders::_1,
                                        std::placeholders::_2));
    groupedPage->setSubmittedPage(
        std::bind(&WEspThingIO::_handleHttpSubmitAddModeProperty, this,
                  std::placeholders::_1, std::placeholders::_2));
    groupedPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(groupedPage);
    // Grouped - Merge
    groupedPage = new WPage(HTTP_ADD_MERGE, "Add/edit Merged Property");
    groupedPage->setShowInMainMenu(false);
    groupedPage->setPrintPage(
        std::bind(&WEspThingIO::_handleHttpAddMergeProperty, this,
                  std::placeholders::_1, std::placeholders::_2));
    groupedPage->setSubmittedPage(
        std::bind(&WEspThingIO::_handleHttpSubmitAddMergeProperty, this,
                  std::placeholders::_1, std::placeholders::_2));
    groupedPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(groupedPage);
    // Remove GPIO
    WPage* removeGpio = new WPage(HTTP_REMOVE_GPIO, "Remove GPIO");
    removeGpio->setShowInMainMenu(false);
    removeGpio->setPrintPage(std::bind(&WEspThingIO::_handleHttpRemoveGpioButton,
                                       this, std::placeholders::_1,
                                       std::placeholders::_2));    
    network->addCustomPage(removeGpio);
  }  

  void _handleHttpUseTemplate(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_USE_TEMPLATE);
    // deviceType
    page->printf(HTTP_COMBOBOX_BEGIN, "Model:", "dt");
    page->printf(HTTP_COMBOBOX_ITEM, "0", HTTP_SELECTED, F("Neo Coolcam"));
    page->printf(HTTP_COMBOBOX_ITEM, "1", "", F("Milos Single Switch"));
    page->printf(HTTP_COMBOBOX_ITEM, "2", "", F("Aubess Touch Switch 2 Gang"));
    page->printf(HTTP_COMBOBOX_ITEM, "3", "", F("Sonoff Mini + Switch"));
    page->printf(HTTP_COMBOBOX_ITEM, "4", "", F("Sonoff Mini + Button"));
    page->printf(HTTP_COMBOBOX_ITEM, "5", "", F("Sonoff Basic"));
    page->printf(HTTP_COMBOBOX_ITEM, "6", "", F("Sonoff 4-channel"));
    page->printf(HTTP_COMBOBOX_ITEM, "7", "", F("Wemos: Relay at D1, Switch at D3"));
    page->print(FPSTR(HTTP_COMBOBOX_END));
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioLed(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_LED);
    _updateEditingItem(request, page);
    // GPIO
    addGpioChooser(page, true, PSTR("GPIO:"), "gp", BYTE_GPIO, 13);
    byte aProps = (_editingItem != nullptr ? _editingItem->getByteArrayValue(BYTE_CONFIG) : 0b00000000);
    _printVisibility(page, aProps, GPIO_TYPE_LED, "led", false);
    // Inverted
    page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv",
                 (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""),
                 "", "Inverted");
    // Linkstate
    page->printf(
        HTTP_CHECKBOX_OPTION, "ls", "ls",
        (bitRead(aProps, BIT_CONFIG_LED_LINKSTATE) ? HTTP_CHECKED : ""), "",
        "Show network link state");
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioRelay(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_RELAY);
    _updateEditingItem(request, page);
    // GPIO
    addGpioChooser(page, true, PSTR("GPIO:"), "gp", BYTE_GPIO, 13);
    byte aProps = (_editingItem != nullptr
                       ? _editingItem->getByteArrayValue(BYTE_CONFIG)
                       : 0b00000110);
    _printVisibility(page, aProps, GPIO_TYPE_RELAY, "on", false);
    // Inverted
    //WHtmlPages::checkBox(page, "iv", "inverted");
    page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv",
                 (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""),
                 "", "Inverted");
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioRgbLed(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_RGB_LED);
    bool newItem = _updateEditingItem(request, page);
    // GPIO
    addGpioChooser(page, true, PSTR("GPIO:"), "gp", BYTE_GPIO, 13);
    // Number of LEDs
    String noLeds((byte)(newItem ? 1 : _editingItem->getByteArrayValue(BYTE_NO_OF_LEDS)));
    WHtml::textField(page, "nol", "Number of LEDs", 3, noLeds.c_str());
    // Properties
    byte aProps = (newItem ? 0b00000110 : _editingItem->getByteArrayValue(BYTE_CONFIG));
    _printVisibility(page, aProps, GPIO_TYPE_RGB_LED, "on", true);
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioHtu21(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_HTU21D);
    bool newItem = _updateEditingItem(request, page);
    addGpioChooser(page, false, PSTR("SDA:"), "sda", BYTE_GPIO, 4);
    addGpioChooser(page, false, PSTR("SCL:"), "scl", BYTE_SCL, 5);
    _handleHttpAddDualProperty(request, page, true, PSTR("Temperature"), PSTR("Humidity"));    
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioSwitch(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_SWITCH);
    bool newItem = _updateEditingItem(request, page);
    byte aProps =
        (newItem ? 0 : _editingItem->getByteArrayValue(BYTE_CONFIG));
    _handleHttpAddGpioSwitchButton(request, page, newItem, aProps);
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioButton(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_BUTTON);
    bool newItem = _updateEditingItem(request, page);
    byte aProps = (newItem ? 0 : _editingItem->getByteArrayValue(BYTE_CONFIG));
    _handleHttpAddGpioSwitchButton(request, page, newItem, aProps);
    // Inverted
    page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv",
                 (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""),
                 "", "Inverted");
    page->printf(
        HTTP_CHECKBOX_OPTION, "lp", "lp",
        (bitRead(aProps, BIT_CONFIG_SWITCH_AP_LONGPRESS) ? HTTP_CHECKED : ""),
        "", "Open AccesPoint on long press");
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddDualProperty(AsyncWebServerRequest* request, Print* page, bool useSecond, const char* firstTitle, const char* secondTitle) {
    bool newItem = _updateEditingItem(request, page);
    byte aProps = (newItem ? 0 : _editingItem->getByteArrayValue(BYTE_CONFIG));
    bool wt = (newItem ? true : bitRead(aProps, BIT_CONFIG_PROPERTY_WEBTHING));
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "wa", (wt ? HTTP_BLOCK : HTTP_NONE), "wb", HTTP_NONE);
    String c = F(" name:");
    String fn = firstTitle; fn.toLowerCase();
    String fnc = firstTitle; fnc.concat(c);
    String sn = secondTitle; sn.toLowerCase();
    String snc = secondTitle; snc.concat(c);
    // Property name
    char* gName = _getSubString(_editingItem, FIRST_NAME);
    WHtml::textField(
        page, "fn", fnc.c_str(), 8,
        (gName != nullptr
             ? gName
             : _getNextName(GPIO_TYPE_MODE, FIRST_NAME, fn).c_str()));
    if (useSecond) {        
      // Mode name
      char* mName = _getSubString(_editingItem, SECOND_NAME);
      WHtml::textField(
          page, "sn", snc.c_str(), 8,
          (mName != nullptr
               ? mName
               : _getNextName(GPIO_TYPE_MODE, SECOND_NAME, sn).c_str()));
    }         
    // showAsWebthingDevice
    page->printf(HTTP_CHECKBOX_OPTION, "wt", "wt", (wt ? HTTP_CHECKED : ""), "tgw()", "Webthing property");
    page->printf(HTTP_DIV_ID_BEGIN, "wa");
    // Webthing title
    c = F(" title:");
    fnc = firstTitle; fnc.concat(c);    
    snc = secondTitle; snc.concat(c);
    char* gTitle = _getSubString(_editingItem, FIRST_TITLE);
    WHtml::textField(
        page, "ft", fnc.c_str(), 12,
        (gTitle != nullptr
             ? gTitle
             : _getNextName(GPIO_TYPE_MODE, FIRST_NAME, firstTitle).c_str()));
    if (useSecond) {                      
      // Mode title
      char* mTitle = _getSubString(_editingItem, SECOND_TITLE);
      WHtml::textField(
          page, "st", snc.c_str(), 12,
          (mTitle != nullptr
               ? mTitle
               : _getNextName(GPIO_TYPE_MODE, SECOND_TITLE, secondTitle).c_str()));
    }         
    page->print(FPSTR(HTTP_DIV_END));
    page->printf(HTTP_TOGGLE_FUNCTION_SCRIPT, "tgw()", "wt", "wa", "wb");
  }

  void _handleHttpAddModeProperty(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_MODE);    
    _handleHttpAddDualProperty(request, page, true, PSTR("Switch"), PSTR("Mode"));    
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddMergeProperty(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_MERGE);
    bool newItem = _updateEditingItem(request, page);
    byte aProps = (newItem ? 0b00000111 : _editingItem->getByteArrayValue(BYTE_CONFIG));
    _printVisibility(page, aProps, GPIO_TYPE_MERGE, "merge", false);
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioSwitchButton(AsyncWebServerRequest* request, Print* page, bool newItem, byte aProps) {
    // GPIO
    addGpioChooser(page, false, PSTR("GPIO:"), "gp", BYTE_GPIO, 13);
    // Output configuration
    bool switchDirectly =
        (newItem ? true : bitRead(aProps, BIT_CONFIG_SWITCH_DIRECTLY));
    // byte linkedGpio = (newItem ? getFirstUnusedGpioOutput() :
    // _editingItem->getByteArrayValue(BYTE_LINKED_GPIO));
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "ma",
                 (switchDirectly ? HTTP_BLOCK : HTTP_NONE), "mb",
                 (!switchDirectly ? HTTP_BLOCK : HTTP_NONE));
    page->printf(HTTP_CHECKBOX_OPTION, "mq", "mq",
                 (switchDirectly ? HTTP_CHECKED : ""), "tg()",
                 "Switch directly");
    // Option A: Switch other GPIO
    page->printf(HTTP_DIV_ID_BEGIN, "ma");
    _addLinkedGpioChooser(page, newItem);
    page->print(FPSTR(HTTP_DIV_END));
    // Option B: Toggle property
    char* gName = _getSubString(_editingItem, FIRST_NAME);
    page->printf(HTTP_DIV_ID_BEGIN, "mb");
    page->printf(HTTP_TEXT_FIELD, "Property name:", "pn", "8",
                 ((gName != nullptr) && (!switchDirectly)
                      ? gName
                      : _getNextName(GPIO_TYPE_SWITCH, FIRST_NAME, "switch")
                            .c_str()));
    page->print(FPSTR(HTTP_DIV_END));
    page->printf(HTTP_TOGGLE_FUNCTION_SCRIPT, "tg()", "mq", "ma", "mb");
  }

  void _handleHttpSubmitUseTemplate(AsyncWebServerRequest* request, Print* page) {
    _configureTemplate(request->arg("dt").toInt());
  }

  void _handleHttpSubmitAddGpioLed(AsyncWebServerRequest* request, Print* page) {
    String voo = request->arg("voo");
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));

    _addLed(request->arg("gp").toInt(), gr, mq, wt,
            (request->arg("iv") == HTTP_TRUE), (!gr ? request->arg("pn") : voo),
            (!gr ? request->arg("wtt") : request->arg("mot")),
            (request->arg("ls") == HTTP_TRUE));
  }

  void _handleHttpSubmitAddGpioRelay(AsyncWebServerRequest* request,
                                    Print* page) {
    String voo = request->arg("voo");
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));
    _addRelay(request->arg("gp").toInt(), gr, mq, wt,
              (request->arg("iv") == HTTP_TRUE),
              (!gr ? request->arg("pn") : voo),
              (!gr ? request->arg("wtt") : request->arg("mot")));
  }

  void _handleHttpSubmitAddGpioRgbLed(AsyncWebServerRequest* request,
                                     Print* page) {
    String voo = request->arg("voo");
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));
    _addRgbLed(request->arg("gp").toInt(), gr, mq, wt,
               request->arg("nol").toInt(), (!gr ? request->arg("pn") : voo),
               (!gr ? request->arg("wtt") : request->arg("mot")),
               (request->arg("gim") == HTTP_TRUE));
  }

  void _handleHttpSubmitAddGpioHtu21(AsyncWebServerRequest* request, Print* page) {    
    _addHtu21D(request->arg("sda").toInt(), request->arg("scl").toInt(), 
               true, (request->arg("wt") == HTTP_TRUE),
               request->arg("fn"), request->arg("ft"),
               request->arg("sn"), request->arg("st"));
  }

  void _handleHttpSubmitAddModeProperty(AsyncWebServerRequest* request, Print* page) {
    _addGroupedProperty((request->arg("wt") == HTTP_TRUE),
                        request->arg("fn"), request->arg("ft"),
                        request->arg("sn"), request->arg("st"));
  }

  void _handleHttpSubmitAddMergeProperty(AsyncWebServerRequest* request, Print* page) {
    String voo = request->arg("voo");
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));
    _addMergedProperty(gr, mq, wt, 
                       request->arg("pn"),
                       (!gr ? request->arg("wtt") : request->arg("mot")),
                       (gr ? voo : ""));
  }

  void _handleHttpSubmitAddGpioSwitch(AsyncWebServerRequest* request,
                                     Print* page) {
    bool switchDirect = (request->arg("mq") == HTTP_TRUE);
    _addSwitch(request->arg("gp").toInt(), switchDirect,
               (switchDirect ? request->arg("lgp") : request->arg("pn")));
  }

  void _handleHttpSubmitAddGpioButton(AsyncWebServerRequest* request,
                                     Print* page) {
    bool switchDirect = (request->arg("mq") == HTTP_TRUE);
    _addButton(request->arg("gp").toInt(), switchDirect,
               (switchDirect ? request->arg("lgp") : request->arg("pn")),
               (request->arg("iv") == HTTP_TRUE),
               (request->arg("lp") == HTTP_TRUE));
  }

  void _handleHttpRemoveGpioButton(AsyncWebServerRequest* request, Print* page) {
    bool exists = (request->hasParam("gpio", true));
    if (exists) {
      String sIndex = request->getParam("gpio", true)->value();
      int index = atoi(sIndex.c_str());
      _removeGpioConfig(index);
      // moveProperty(index + 1, MAX_GPIOS);
      page->print("Deleted item: ");
      page->print(sIndex);
    } else {
      page->print("Missing GPIO value. Nothing deleted.");
    }
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, "Go back");
  }

  virtual void _printConfigPage(AsyncWebServerRequest* request, Print* page) {
    if (_numberOfGPIOs->getByte() < MAX_GPIOS) {
      page->print(F("<table  class='tt'>"));
      tr(page);
      page->print(F("<td colspan='4'>"));
      page->printf(HTTP_BUTTON, HTTP_USE_TEMPLATE, VALUE_GET, "Use a template...");
      tdEnd(page);
      trEnd(page);
      tr(page);
      td(page);
      page->print(F("Add output:"));
      tdEnd(page);
      // LED
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_LED, VALUE_GET, "LED");
      tdEnd(page);
      // Relay
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_RELAY, VALUE_GET, "Relay");
      tdEnd(page);
      // Relay
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_RGB_LED, VALUE_GET, "RGB LED");
      tdEnd(page);
      // Switch
      trEnd(page);
      tr(page);
      td(page);
      page->print(F("Add input:"));
      tdEnd(page);
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_SWITCH, VALUE_GET, "Switch");
      tdEnd(page);
      // Button
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_BUTTON, VALUE_GET, "Button");
      tdEnd(page);
      // HTU21D
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_HTU21D, VALUE_GET, "HTU21D");
      tdEnd(page);
      trEnd(page);
      tr(page);
      td(page);
      page->print(F("Add grouped:"));
      tdEnd(page);
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_MODE, VALUE_GET, "On/Mode");
      tdEnd(page);
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_MERGE, VALUE_GET, "Merge");
      tdEnd(page);
      trEnd(page);
      page->print(F("</table>"));
    }
    // Table with already stored MAX_GPIOS
    page->printf(HTTP_DIV_ID_BEGIN, "gd");
    page->print(F("<table  class='st'>"));
    tr(page);
    th(page); /*No*/
    thEnd(page);
    th(page);
    page->print("Type");
    thEnd(page);
    th(page);
    page->print("GPIO");
    thEnd(page);
    th(page);
    page->print("Name");
    thEnd(page);
    th(page); /*Edit*/
    thEnd(page);
    th(page); /*Remove*/
    thEnd(page);
    trEnd(page);
    char* pName = new char[8];

    char* pNumber = new char[2];
    char* gNumber = new char[2];
    for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte aProps = gConfig->getByteArrayValue(BYTE_CONFIG);
      bool gr = bitRead(aProps, BIT_CONFIG_PROPERTY_GROUPED);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      sprintf(pName, "gpio_%d", i);
      sprintf(pNumber, "%d", i);
      tr(page);
      td(page);
      page->print(pNumber);
      tdEnd(page);
      td(page);      
      page->print(_getGpioDisplayName(gType));
      char* targetName = _getGpioTarget(gType);
      tdEnd(page);
      td(page);
      if (!_isGpioGrouped(gType)) {
        sprintf(gNumber, "%d", gConfig->getByteArrayValue(BYTE_GPIO));
        page->print(gNumber);
      }
      tdEnd(page);
      td(page);
      if (gr) {
        page->print("> ");
      }
      char* gName = (gr && (gType == GPIO_TYPE_MERGE) ? _getSubString(gConfig, SECOND_NAME) : _getSubString(gConfig, FIRST_NAME));
      if (gName != nullptr) {
        page->print(gName);
      }
      tdEnd(page);
      td(page);
      page->printf(HTTP_BUTTON_VALUE, targetName, "", pNumber, CAPTION_EDIT);
      tdEnd(page);
      td(page);
      page->printf(HTTP_BUTTON_VALUE, HTTP_REMOVE_GPIO, "cbtn", pNumber,
                   CAPTION_REMOVE);
      tdEnd(page);
      trEnd(page);
    }
    page->print(F("</table>"));
    page->printf(HTTP_DIV_END);

    page->printf(HTTP_CONFIG_PAGE_BEGIN, id());
    page->print(FPSTR(HTTP_CONFIG_SAVE_BUTTON));
  }

  void _saveConfigPage(AsyncWebServerRequest* request, Print* page) {}

 protected:
  WProperty* _getGpioConfig(byte index) {
    String pNumber = GPIO_DEFAULT_ID;
    pNumber.concat(index);
    return network()->settings()->getSetting(pNumber.c_str());
  }

  WProperty* _getGroupedGpioByName(const char* name) {
    return _getGroupedGpioBySubString(name, FIRST_NAME);
  }

  WProperty* _getGroupedGpioByModeName(const char* name) {
    return _getGroupedGpioBySubString(name, SECOND_NAME);
  }

  WProperty* _getGroupedGpioBySubString(const char* name, byte subStringIndex) {
    for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      if (_isGpioGrouped(gType)) {
        char* gName = _getSubString(gConfig, subStringIndex);
        if ((gName != nullptr) && (strcmp(gName, name) == 0)) {
          return gConfig;
        }
      }
    }
    return nullptr;
  }

  WProperty* _addGpioConfig(byte gType) {
    byte index = _numberOfGPIOs->getByte();
    String pNumber = GPIO_DEFAULT_ID;
    pNumber.concat(index);
    // Config
    // WProperty* gConfig = network()->settings()->setByteArray(pNumber.c_str(),
    // DEFAULT_PROP_ARRAY);
    WProperty* gConfig = new WProperty(pNumber.c_str(), pNumber.c_str(), BYTE_ARRAY, "");
    gConfig->setByteArray(DEFAULT_PROP_ARRAY);
    network()->settings()->insert(gConfig, _numberOfSettings);    
    _numberOfSettings++;
    if (gType == GPIO_TYPE_UNKNOWN) {
      gType = gConfig->getByteArrayValue(BYTE_TYPE);
    } else {
      gConfig->setByteArrayValue(BYTE_TYPE, gType);
    }
    pNumber.concat(".");
    for (byte i = 0; i < _getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);
      // network()->settings()->setString(subNumber.c_str(), "");
      WProperty* sp = WProps::createStringProperty(subNumber.c_str(), "");
      network()->settings()->insert(sp, _numberOfSettings);
      _numberOfSettings++;
    }
    return gConfig;
  }

  void _removeGpioConfig(byte index) {
    WProperty* gConfig = _getGpioConfig(index);
    byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
    String pNumber = GPIO_DEFAULT_ID;
    pNumber.concat(index);
    // Config
    network()->settings()->remove(pNumber.c_str());
    _numberOfSettings--;
    pNumber.concat(".");
    for (byte i = 0; i < _getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);
      network()->settings()->remove(subNumber.c_str());
      _numberOfSettings--;
    }
    // Move next gpios 1 place forward

    for (int i = index + 1; i < _numberOfGPIOs->getByte(); i++) {
      gConfig = _getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      // Config
      pNumber = GPIO_DEFAULT_ID;
      pNumber.concat(i);
      String p2 = GPIO_DEFAULT_ID;
      p2.concat(i - 1);
      network()->settings()->getSetting(pNumber.c_str())->setId(p2.c_str());
      pNumber.concat(".");
      p2.concat(".");
      for (byte b = 0; b < _getNumberOfChars(gType); b++) {
        String subNumber = pNumber;
        subNumber.concat(b);
        String subP2 = p2;
        subP2.concat(b);
        network()
            ->settings()
            ->getSetting(subNumber.c_str())
            ->setId(subP2.c_str());
      }
    }
    _numberOfGPIOs->setByte(_numberOfGPIOs->getByte() - 1);
  }

  bool _updateEditingItem(AsyncWebServerRequest* request, Print* page) {
    bool exists = (request->hasParam("gpio", true));
    if (!exists) {
      page->print("New item");
      _clearEditingItem();
      return true;
    } else {
      String sIndex = request->getParam("gpio", true)->value();
      int index = atoi(sIndex.c_str());
      page->print("Edit item: ");
      page->print(sIndex);
      _editingIndex = index;
      _editingItem = _getGpioConfig(index);
      return false;
    }
  }

  void _ensureEditingItem(byte gType) {
    if (_editingIndex == NO_INDEX) {
      _editingIndex = _numberOfGPIOs->getByte();
      _editingItem = _addGpioConfig(gType);
      _numberOfGPIOs->setByte(_editingIndex + 1);
    }
  }

  bool _isGpioFree(byte gpio) {
    // exclude already used GPIOs
    if ((_editingItem != nullptr) && (_editingItem->getByteArrayValue(BYTE_GPIO) == gpio)) {
      return true;
    } else {
      for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
        if ((_getGpioConfig(i)->getByteArrayValue(BYTE_GPIO) == gpio) || (_getGpioConfig(i)->getByteArrayValue(BYTE_SCL) == gpio)) {
          return false;
        }
      }
    }
    return true;
  }

  bool _isGpioAnOutput(byte gType) {
    return ((gType == GPIO_TYPE_LED) || (gType == GPIO_TYPE_RELAY) ||
            (gType == GPIO_TYPE_RGB_LED));
  }

  bool _isGpioAnInput(byte gType) {
    return ((gType == GPIO_TYPE_BUTTON) || (gType == GPIO_TYPE_SWITCH) ||
            (gType == GPIO_TYPE_HTU21D));
  }

  bool _isGpioGrouped(byte gType) {
    return ((gType == GPIO_TYPE_MODE) || (gType == GPIO_TYPE_MERGE));
  }

  bool _addGpioChooserItem(Print* page, byte gpio, const char* title, byte bytePos, byte defaultGpio) {
    int aGpio = (_editingItem != nullptr ? _editingItem->getByteArrayValue(bytePos) : defaultGpio);
    char* pNumber = new char[2];
    sprintf(pNumber, "%d", gpio);
    if (_isGpioFree(gpio))
      page->printf(HTTP_COMBOBOX_ITEM, pNumber, (aGpio == gpio ? HTTP_SELECTED : ""), title);
    return true;
  }

  void addGpioChooser(Print* page, bool isOutput, const char* title, const char* fieldName, byte bytePos, byte defaultGpio) {
    page->printf(HTTP_COMBOBOX_BEGIN, title, fieldName);
#ifdef ESP8266
    _addGpioChooserItem(page, 0, "0", bytePos, defaultGpio);
    if (isOutput) _addGpioChooserItem(page, 1, "1 (TX)", bytePos, defaultGpio);
    _addGpioChooserItem(page, 2, "2", bytePos, defaultGpio);
    if (!isOutput) _addGpioChooserItem(page, 3, "3 (RX)", bytePos, defaultGpio);
    _addGpioChooserItem(page, 4, "4", bytePos, defaultGpio);
    _addGpioChooserItem(page, 5, "5", bytePos, defaultGpio);
    _addGpioChooserItem(page, 12, "12", bytePos, defaultGpio);
    _addGpioChooserItem(page, 13, "13", bytePos, defaultGpio);
    _addGpioChooserItem(page, 14, "14", bytePos, defaultGpio);
    if (isOutput) _addGpioChooserItem(page, 15, "15", bytePos, defaultGpio);
#elif ESP32
    if (isOutput) {
      _addGpioChooserItem(page, 0, "0", bytePos, defaultGpio);
      _addGpioChooserItem(page, 1, "1 (TX)", bytePos, defaultGpio);
    }
    _addGpioChooserItem(page, 2, "2", bytePos, defaultGpio);
    if (!isOutput) {
      _addGpioChooserItem(page, 3, "3 (RX)", bytePos, defaultGpio);
    }
    _addGpioChooserItem(page, 4, "4", bytePos, defaultGpio);
    _addGpioChooserItem(page, 5, "5", bytePos, defaultGpio);
    _addGpioChooserItem(page, 12, "12", bytePos, defaultGpio);
    _addGpioChooserItem(page, 13, "13", bytePos, defaultGpio);
    _addGpioChooserItem(page, 14, "14", bytePos, defaultGpio);
    _addGpioChooserItem(page, 15, "15", bytePos, defaultGpio);
    _addGpioChooserItem(page, 16, "16", bytePos, defaultGpio);
    _addGpioChooserItem(page, 17, "17", bytePos, defaultGpio);
    _addGpioChooserItem(page, 18, "18", bytePos, defaultGpio);
    _addGpioChooserItem(page, 19, "19", bytePos, defaultGpio);
    _addGpioChooserItem(page, 21, "21", bytePos, defaultGpio);
    _addGpioChooserItem(page, 22, "22", bytePos, defaultGpio);
    _addGpioChooserItem(page, 23, "23", bytePos, defaultGpio);
    _addGpioChooserItem(page, 25, "25", bytePos, defaultGpio);
    _addGpioChooserItem(page, 26, "26", bytePos, defaultGpio);
    _addGpioChooserItem(page, 27, "27", bytePos, defaultGpio);
    _addGpioChooserItem(page, 32, "32", bytePos, defaultGpio);
    _addGpioChooserItem(page, 33, "33", bytePos, defaultGpio);
    if (!isOutput) {
      _addGpioChooserItem(page, 34, "34", bytePos, defaultGpio);
      _addGpioChooserItem(page, 35, "35", bytePos, defaultGpio);
      _addGpioChooserItem(page, 36, "36", bytePos, defaultGpio);
      _addGpioChooserItem(page, 39, "39", bytePos, defaultGpio);
    }
#endif
    page->print(FPSTR(HTTP_COMBOBOX_END));
  }

  char* _getGpioDisplayName(byte gType) {
    switch (gType) {
      case GPIO_TYPE_LED:
        return "LED";
      case GPIO_TYPE_RELAY:
        return "Relay";
      case GPIO_TYPE_BUTTON:
        return "Button";
      case GPIO_TYPE_SWITCH:
        return "Switch";
      case GPIO_TYPE_MODE:
        return "On/Mode";
      case GPIO_TYPE_MERGE:
        return "Merge";
      case GPIO_TYPE_RGB_LED:
        return "RGB";
      case GPIO_TYPE_HTU21D:
        return "HTU21D";  
      default:
        return "n.a.";
    }
  }

  char* _getGpioTarget(byte gType) {
    switch (gType) {
      case GPIO_TYPE_LED:
        return HTTP_ADD_LED;
      case GPIO_TYPE_RELAY:
        return HTTP_ADD_RELAY;
      case GPIO_TYPE_BUTTON:
        return HTTP_ADD_BUTTON;
      case GPIO_TYPE_SWITCH:
        return HTTP_ADD_SWITCH;
      case GPIO_TYPE_MODE:
        return HTTP_ADD_MODE;
      case GPIO_TYPE_MERGE:
        return HTTP_ADD_MERGE;
      case GPIO_TYPE_RGB_LED:
        return HTTP_ADD_RGB_LED;
      case GPIO_TYPE_HTU21D:
        return HTTP_ADD_HTU21D;  
      default:
        return HTTP_ADD_LED;
    }
  }

  String _getNextName(byte aType, byte subIndex, String baseName) {
    int noMax = 0;
    for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      byte aProps = gConfig->getByteArrayValue(BYTE_CONFIG);
      bool gr = bitRead(aProps, BIT_CONFIG_PROPERTY_GROUPED);
      if (aType == gType) {
        int b = -1;
        String gName = "";
        if (!gr) {
          char* gN = _getSubString(gConfig, subIndex);
          gName = String(gN);
          b = gName.lastIndexOf("_");
        }
        if (b > -1) {
          noMax = max(noMax, (int)gName.substring(b + 1, gName.length()).toInt());
          baseName = gName.substring(0, b);
        } else {
          noMax = 1;
          baseName = (gName.length() > 0 ? gName : baseName);
        }
      }
    }
    if (noMax > 0) {
      baseName.concat("_");
      baseName.concat(String(noMax + 1));
    }
    return baseName;
  }

  void _addLinkedGpioChooser(Print* page, boolean newItem) {
    // BYTE_SWITCH_LINKED_GPIO
    page->printf(HTTP_COMBOBOX_BEGIN, "Select output GPIO:", "lgp");
    WProperty* linkedName =
        (newItem ? nullptr : _getSubProperty(_editingItem, FIRST_NAME));
    // page->printf(HTTP_COMBOBOX_ITEM, "255", (BYTE_NOT_LINKED_GPIO ==
    // linkedGpio ? HTTP_SELECTED : ""), "None");
    for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      byte gGpio = gConfig->getByteArrayValue(BYTE_GPIO);
      char* gName = _getSubString(gConfig, FIRST_NAME);
      byte aProps = gConfig->getByteArrayValue(BYTE_CONFIG);
      bool mq = bitRead(aProps, BIT_CONFIG_PROPERTY_MQTT);
      if ((mq) && ((_isGpioAnOutput(gType)) || (_isGpioGrouped(gType)))) {
        String iName = String(_getGpioDisplayName(gType));
        iName.concat(" (");
        iName.concat(gName);
        iName.concat(")");
        page->printf(
            HTTP_COMBOBOX_ITEM, gName,
            ((linkedName != nullptr) && (linkedName->equalsString(gName))
                 ? HTTP_SELECTED
                 : ""),
            iName.c_str());
      }
    }
    page->print(FPSTR(HTTP_COMBOBOX_END));
  }

  void _printVisibility(Print* page, byte aProps, byte aType, String baseName, bool groupedInModeCheckBox) {
    bool gr = bitRead(aProps, BIT_CONFIG_PROPERTY_GROUPED);    
    bool mq = bitRead(aProps, BIT_CONFIG_PROPERTY_MQTT);
    bool wt = bitRead(aProps, BIT_CONFIG_PROPERTY_WEBTHING);
    char* eName = _getSubString(_editingItem, FIRST_NAME);
    if (aType == GPIO_TYPE_MERGE) {
      WHtml::textField(page, "pn", "Property name:", 8, (eName != nullptr ? eName : _getNextName(aType, FIRST_NAME, baseName).c_str()));
      eName = _getSubString(_editingItem, SECOND_NAME);
    }
    // ComboBox
    page->printf(HTTP_COMBOBOX_OPTION_BEGIN, "Property:", "voo");
    // MQTT and Webthings
    WHtml::comboBoxItem(page, "MQTT and Webthings", "2", !gr && mq && wt);
    // MQTT
    WHtml::comboBoxItem(page, "Only MQTT", "1", !gr && mq && !wt);
    // None
    if (aType != GPIO_TYPE_MERGE) {
      WHtml::comboBoxItem(page, "No property", "0", !gr && !mq && !wt);
    }          
    // List grouped properties
    for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      if (_isGpioGrouped(gType) && (gType != aType)) {
        char* gName = _getSubString(gConfig, FIRST_NAME);
        String iName = String(_getGpioDisplayName(gType));
        iName.concat(" (");
        iName.concat(gName);
        iName.concat(")");
        // char* gTitle = _getSubString(gConfig, PROPERTY_TITLE);
        bool selected = (gr) && (eName != nullptr) && (gName != nullptr) && (strcmp(eName, gName) == 0);
        WHtml::comboBoxItem(page, iName.c_str(), gName, selected);
      }
    }
    page->print(FPSTR(HTTP_COMBOBOX_END));
    page->print(FPSTR(HTTP_COMBO_BOX_FUNCTION_SCRIPT));  
    // MQTT property name
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "ma", (((aType == GPIO_TYPE_MERGE) || (!gr && mq)) ? HTTP_BLOCK : HTTP_NONE), "mb", HTTP_NONE);                 
    page->printf(HTTP_DIV_ID_BEGIN, "ma");
    if (aType != GPIO_TYPE_MERGE) {
      WHtml::textField(page, "pn", "Property name:", 8, (eName != nullptr ? eName : _getNextName(aType, FIRST_NAME, baseName).c_str()));
    }
    page->print(FPSTR(HTTP_DIV_END));
    // Webthing title
    char* eTitle = _getSubString(_editingItem, FIRST_TITLE);
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "wa", (!gr && wt ? HTTP_BLOCK : HTTP_NONE), "wb", HTTP_NONE);
    page->printf(HTTP_DIV_ID_BEGIN, "wa");
    WHtml::textField(
        page, "wtt", "Webthing title:", 12,
        (eTitle != nullptr
             ? eTitle
             : _getNextName(aType, FIRST_TITLE, baseName).c_str()));
    page->print(FPSTR(HTTP_DIV_END));
    // Mode name
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "na", (gr ? HTTP_BLOCK : HTTP_NONE), "nb", HTTP_NONE);
    page->printf(HTTP_DIV_ID_BEGIN, "na");
    WHtml::textField(page, "mot", "Mode name:", 12, (eTitle != nullptr ? eTitle : _getNextName(aType, FIRST_NAME, baseName).c_str()));
    if (groupedInModeCheckBox) {
      //BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE
      bool gim = bitRead(aProps, BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE);
      WHtml::checkBox(page, "gim", PSTR("Add different modes in dropdown list"), gim);      
    }
    page->print(FPSTR(HTTP_DIV_END));
  }

 private:
  WProperty* _numberOfGPIOs;
  byte _numberOfSettings;
  WProperty* _editingItem;
  byte _editingIndex;

  void _configureOutput(WOutput* output, WProperty* gConfig, bool gr, bool mq, bool wt) {
    this->addOutput(output);    
    char* gName = _getSubString(gConfig, FIRST_NAME);
    char* gTitle = _getSubString(gConfig, FIRST_TITLE);
    if (gr) {      
      WProperty* groupedGpio = _getGroupedGpioByName(gName);
      if (groupedGpio != nullptr) {
        if (groupedGpio->getByteArrayValue(BYTE_TYPE) == GPIO_TYPE_MODE) {
          char* modeName = _getSubString(groupedGpio, SECOND_NAME);
          if (modeName != nullptr) {
            WProperty* modeProp = this->getPropertyById(modeName);
            if (modeProp != nullptr) {
              byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
              bool pim = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE);
              if ((pim) && (output->countModes() > 0)) {
                for (byte i = 0; i < output->countModes(); i++) {
                  String mTitle = String(gTitle);
                  mTitle.concat(" - ");
                  mTitle.concat(output->modeTitle(i));
                  modeProp->addEnumString(mTitle.c_str());
                }
              } else {
                modeProp->addEnumString(gTitle);
                if (modeProp->isNull()) {
                  modeProp->setString(gTitle);
                }
                _configureOutputModes(output, gConfig, modeProp->isVisible(MQTT), modeProp->isVisible(WEBTHING));  
              }
            }
          }
        } 
        groupedGpio->addOutput(output);
        output->setId(gTitle);
        network()->debug(F("grouped %s has added an output"), groupedGpio->id(), output->id());
      } else {
        network()->error("Can't find grouped property. Name of grouped property: '%s'", gName);
      }
    } else if ((mq) || (wt)) {
      WProperty* relayProp = WProps::createOnOffProperty(gName, gTitle);
      relayProp->setVisibilityMqtt(mq);
      relayProp->setVisibilityWebthing(wt);
      relayProp->addOutput(output);
      this->addProperty(relayProp);
      _configureOutputModes(output, gConfig, mq, wt);      
    }
  }

  void _configureOutputModes(WOutput* output, WProperty* gConfig, bool mq, bool wt) {
    if (output->countModes() > 1) {      
      //Mode property
      String mName = String(_getSubString(gConfig, FIRST_NAME));
      mName.concat(F("_mode"));
      String mTitle = String(_getSubString(gConfig, FIRST_TITLE));
      mTitle.concat(F(" Mode"));
      WProperty* modeProp = WProps::createStringProperty(mName.c_str(), mTitle.c_str());
      network()->settings()->add(modeProp);   
      this->addProperty(modeProp);     
      modeProp->setVisibility(mq, wt); 
      //Enums
      for (byte i = 0; i < output->countModes(); i++) {
        modeProp->addEnumString(output->modeTitle(i));
      }
      //Default/Initial value
      if (!modeProp->equalsString("")) {
        output->setModeByTitle(modeProp->c_str());
      } else {
        modeProp->setString(output->modeTitle(output->mode()));
      }
      //Change notification
      modeProp->setOnChange([this, output] (WProperty* property) {
        network()->settings()->save();
        output->setModeByTitle(property->c_str());
      });
    }
  }  

  void _notifyGroupedChange(WProperty* property, byte subStringIndex) {
    WProperty* groupedGpio = _getGroupedGpioBySubString(property->id(), subStringIndex);
    if (groupedGpio != nullptr) {
      char* gName = _getSubString(groupedGpio, FIRST_NAME);
      char* mName = _getSubString(groupedGpio, SECOND_NAME);
      if ((gName != nullptr) && (mName != nullptr)) {
        WProperty* onOffProp = this->getPropertyById(gName);
        WProperty* modeProp = this->getPropertyById(mName);
        if ((onOffProp != nullptr) && (modeProp != nullptr)) {
          if (groupedGpio->hasOutputs()) {
            groupedGpio->outputs()->forEach(
                [this, onOffProp, modeProp](WOutput* output) {
                  String outputName = String(modeProp->c_str());
                  int mIndex = outputName.indexOf(" - ");
                  if (mIndex > -1) {
                    String modeName = outputName.substring(mIndex + 3);
                    outputName = outputName.substring(0, mIndex);
                    network()->debug("Look for mode '%s', output name is '%s'", modeName, outputName);
                    output->setModeByTitle(modeName.c_str());
                  } else {
                    network()->debug("Switch output '%s'", outputName);
                  }                  
                  output->setOn((onOffProp->getBoolean()) && (output->equalsId(outputName.c_str())));
                });
          } else {
            network()->error(F("Grouped GPIO '%s' has no outputs"), groupedGpio->id());
          }
        } else {
          network()->error(F("Name property or mode property is missing for grouped property: '%s'"), property->c_str());
        }
      } else {
        network()->error(F("Name or mode name is missing for grouped property: '%s'"), property->c_str());
      }
    } else {
      network()->error(F("Can't find grouped property. Name of grouped property: '%s'"), property->c_str());
    }
  }

  void _notifyMergedChange(WProperty* property) {
    WProperty* mergedGpio =  _getGroupedGpioBySubString(property->id(), FIRST_NAME);
    if (mergedGpio != nullptr) {
      char* gName = _getSubString(mergedGpio, FIRST_NAME);
      if (mergedGpio->hasOutputs()) {        
        mergedGpio->outputs()->forEach([this, property](WOutput* output) {          
          output->setOn(property->getBoolean());
        });
      }
    } else {
      network()->error(F("Can't find merged property. Name of merged property: '%s'"), property->c_str());
    }
  }

  void _configureDevice() {
    // 1. Grouped
    for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      if (gType == GPIO_TYPE_MODE) {
        char* gName = _getSubString(gConfig, FIRST_NAME);
        char* gTitle = _getSubString(gConfig, FIRST_TITLE);
        char* mName = _getSubString(gConfig, SECOND_NAME);
        network()->debug(F("add mode item '%s'"), gName);
        char* mTitle = _getSubString(gConfig, SECOND_TITLE);
        bool mq = true;
        bool wt = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
        WProperty* onOffProp = WProps::createOnOffProperty(gName, gTitle);
        onOffProp->setVisibilityMqtt(mq);
        onOffProp->setVisibilityWebthing(wt);
        onOffProp->setOnChange([this](WProperty* property) { _notifyGroupedChange(property, FIRST_NAME); });
        this->addProperty(onOffProp);
        WProperty* modeProp = WProps::createStringProperty(mName, mTitle);
        network()->settings()->add(modeProp);        
        modeProp->setVisibilityMqtt(mq);
        modeProp->setVisibilityWebthing(wt);
        modeProp->setOnChange([this](WProperty* property) {
          network()->settings()->save();
          _notifyGroupedChange(property, SECOND_NAME);
        });
        this->addProperty(modeProp);
      }
    }    
    // 2. Merged
    for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      if (gType == GPIO_TYPE_MERGE) {
        char* gName = _getSubString(gConfig, FIRST_NAME);
        char* gTitle = _getSubString(gConfig, FIRST_TITLE);
        network()->debug(F("add merge item '%s'"), gName);
        bool mq = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT);
        bool wt = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
        bool gr = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_GROUPED);
        WProperty* onOffProp = WProps::createOnOffProperty(gName, gTitle);
        onOffProp->setVisibilityMqtt(mq);
        onOffProp->setVisibilityWebthing(wt);
        onOffProp->setOnChange([this](WProperty* property) { _notifyMergedChange(property); });
        this->addProperty(onOffProp);              
        if (gr) {
          char* pgName = _getSubString(gConfig, SECOND_NAME);          
          WProperty* groupedGpio = _getGroupedGpioByName(pgName);
          if (groupedGpio != nullptr) {
            if (groupedGpio->getByteArrayValue(BYTE_TYPE) == GPIO_TYPE_MODE) {
              char* modeName = _getSubString(groupedGpio, SECOND_NAME);                            
              if (modeName != nullptr) {
                WProperty* modeProp = this->getPropertyById(modeName);
                if (modeProp != nullptr) {
                  modeProp->addEnumString(gTitle);
                  if (modeProp->isNull()) {
                    modeProp->setString(gTitle);
                  }
                }
              }
            } 
            WMergedOutput* output = new WMergedOutput(onOffProp);
            groupedGpio->addOutput(output);
            output->setId(gTitle);
            network()->debug("grouped %s has added a merged output", groupedGpio->id(), output->id());
          } else {
            network()->error("Can't find grouped property. Name of grouped property: '%s'", pgName);
          }
        }         
      }
    }        
    // 3. Outputs
    for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      bool gr = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_GROUPED);
      bool mq = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT);
      bool wt = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
      switch (gConfig->getByteArrayValue(BYTE_TYPE)) {
        case GPIO_TYPE_LED: {
          network()->debug(F("add led"));
          WLed* led = new WLed(gConfig->getByteArrayValue(BYTE_GPIO));
          _configureOutput(led, gConfig, gr, mq, wt);
          // Inverted
          led->setInverted(gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
          // Linkstate
          if (gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_LINKSTATE)) {
            network()->setStatusLed(led, false);
          }
          break;
        }
        case GPIO_TYPE_RELAY: {
          network()->debug(F("add relay"));
          WRelay* relay = new WRelay(gConfig->getByteArrayValue(BYTE_GPIO), gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
          _configureOutput(relay, gConfig, gr, mq, wt);
          break;
        }
        case GPIO_TYPE_RGB_LED: {
          network()->debug(F("add RGB led"));
          W2812Led* ledStrip = new W2812Led(network(), gConfig->getByteArrayValue(BYTE_GPIO), gConfig->getByteArrayValue(BYTE_NO_OF_LEDS));          
          _configureOutput(ledStrip, gConfig, gr, mq, wt);
          //ledStrip->led
          if (!gr) {

          }
          break;
        }
      }
    }
    // 4. Inputs
    for (byte i = 0; i < _numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      char* gName = _getSubString(gConfig, FIRST_NAME);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);      
      switch (gType) {
        case GPIO_TYPE_BUTTON:
        case GPIO_TYPE_SWITCH: {          
          network()->debug(F("add button/switch '%s'"), gName);
          bool inverted = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED);
          bool longPress = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_AP_LONGPRESS);
          byte bMode = (gType == GPIO_TYPE_SWITCH ? MODE_SWITCH : (longPress ? MODE_BUTTON_LONG_PRESS : MODE_BUTTON));
          WSwitch* button = new WSwitch(gConfig->getByteArrayValue(BYTE_GPIO), bMode, inverted);
          if (longPress) {
            button->setOnLongPressed([this]() {
              if (!network()->isSoftAP()) {
                network()->settings()->forceAPNextStart();
                delay(200);
                ESP.restart();
              }
            });
          }
          this->addInput(button);
          bool switchDirect = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_DIRECTLY);
          if (switchDirect) {               
            button->setProperty(this->getPropertyById(gName));
          } else if ((gName != nullptr) && (strcmp(gName, "") != 0)) {
            // Only trigger via MQTT
            WProperty* triggerProperty = WProps::createOnOffProperty(gName, gName);
            triggerProperty->setVisibility(MQTT);
            this->addProperty(triggerProperty);
            button->setTriggerProperty(triggerProperty);            
          }
          break;
        }
        case GPIO_TYPE_HTU21D: {
          network()->debug(F("add HTU21 temperature sensor '%s'"), gName);
          //Sensor          
          WHtu21D* htu = new WHtu21D(gConfig->getByteArrayValue(BYTE_GPIO), gConfig->getByteArrayValue(BYTE_SCL));                    
          this->addInput(htu);
          //Properties
          bool mq = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT);
          bool wt = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
          char* gTitle = _getSubString(gConfig, FIRST_TITLE);
          WProperty* temperature = WProps::createTemperatureProperty(gName, gTitle);
          temperature->setVisibility(mq, wt);          
          this->addProperty(temperature);    
          gName = _getSubString(gConfig, SECOND_NAME);
          gTitle = _getSubString(gConfig, SECOND_TITLE);
          WProperty* humidity = WProps::createHumidityProperty(gName, gTitle);          
          this->addProperty(humidity);
          //Set properties to sensor
          htu->setProperty(temperature);          
          htu->setHumidity(humidity);
          break;
        }
      }
    }
  }

  void _addGpioConfigItem(byte type, byte gpio, bool grouped, bool mqtt, bool webthing, bool inverted) {
    _ensureEditingItem(type);
    //_editingItem->setByteArrayValue(BYTE_TYPE, type);
    _editingItem->setByteArrayValue(BYTE_GPIO, gpio);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_GROUPED, grouped);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT, mqtt);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING, webthing);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED, inverted);

    String pNumber = _editingItem->id();
    pNumber.concat(".");
    byte gType = _editingItem->getByteArrayValue(BYTE_TYPE);
    for (byte i = 0; i < _getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);
      network()->settings()->setString(subNumber.c_str(), "");
    }
  }

  void _clearEditingItem() {
    _editingItem = nullptr;
    _editingIndex = NO_INDEX;
  }

  WProperty* _setSubString(WProperty* gpioConfig, byte subIndex, String value) {
    String pNumber = gpioConfig->id();
    pNumber.concat(".");
    pNumber.concat(subIndex);
    return network()->settings()->setString(pNumber.c_str(), value.c_str());
  }

  WProperty* _getSubProperty(WProperty* gpioConfig, byte subIndex) {
    if (gpioConfig != nullptr) {
      String pNumber = gpioConfig->id();
      pNumber.concat(".");
      pNumber.concat(subIndex);
      return network()->settings()->getSetting(pNumber.c_str());
    } else {
      return nullptr;
    }
  }

  char* _getSubString(WProperty* gpioConfig, byte subIndex) {
    WProperty* p = _getSubProperty(gpioConfig, subIndex);
    return (p != nullptr ? p->c_str() : nullptr);
  }

  void _addRelay(byte gpio, bool gr, bool mqtt, bool webthing, bool inverted, String oName, String oTitle) {
    _addGpioConfigItem(GPIO_TYPE_RELAY, gpio, gr, mqtt, webthing, inverted);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _setSubString(_editingItem, FIRST_TITLE, oTitle);
    _clearEditingItem();
  }

  void _addLed(byte gpio, bool gr, bool mqtt, bool webthing, bool inverted, String oName, String oTitle, bool linkState) {
    _addGpioConfigItem(GPIO_TYPE_LED, gpio, gr, mqtt, webthing, inverted);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _setSubString(_editingItem, FIRST_TITLE, oTitle);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_LINKSTATE, linkState);
    _clearEditingItem();
  }

  void _addRgbLed(byte gpio, bool gr, bool mqtt, bool webthing, byte noOfLeds, String oName, String oTitle, bool groupInMode) {
    _addGpioConfigItem(GPIO_TYPE_RGB_LED, gpio, gr, mqtt, webthing, false);
    _editingItem->setByteArrayValue(BYTE_NO_OF_LEDS, noOfLeds);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _setSubString(_editingItem, FIRST_TITLE, oTitle);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE, groupInMode);
    _clearEditingItem();
  }

  void _addHtu21D(byte sda, byte scl, bool mqtt, bool webthing, String tName, String tTitle, String hName, String hTitle) {
    _addGpioConfigItem(GPIO_TYPE_HTU21D, sda, false, mqtt, webthing, false);
    _editingItem->setByteArrayValue(BYTE_SCL, scl);
    _setSubString(_editingItem, FIRST_NAME, tName);
    _setSubString(_editingItem, FIRST_TITLE, tTitle);
    _setSubString(_editingItem, SECOND_NAME, hName);
    _setSubString(_editingItem, SECOND_TITLE, hTitle);
    _clearEditingItem();
  }

  void _addGroupedProperty(bool webthing, String propertyName, String propertyTitle, String modeName, String modeTitle) {
    _addGpioConfigItem(GPIO_TYPE_MODE, BYTE_NO_GPIO, false, true, webthing, false);
    _setSubString(_editingItem, FIRST_NAME, propertyName);
    _setSubString(_editingItem, FIRST_TITLE, propertyTitle);
    _setSubString(_editingItem, SECOND_NAME, modeName);
    _setSubString(_editingItem, SECOND_TITLE, modeTitle);
    _clearEditingItem();
  }

  void _addMergedProperty(bool gr, bool mqtt, bool webthing, String propertyName, String propertyTitle, String groupedName) {
    _addGpioConfigItem(GPIO_TYPE_MERGE, BYTE_NO_GPIO, gr, mqtt, webthing, false);
    _setSubString(_editingItem, FIRST_NAME, propertyName);
    _setSubString(_editingItem, FIRST_TITLE, propertyTitle);    
    _setSubString(_editingItem, SECOND_NAME, groupedName);    
    _clearEditingItem();
  }

  void _addSwitch(byte gpio, bool switchDirect, String oName) {
    _addGpioConfigItem(GPIO_TYPE_SWITCH, gpio, false, false, false, false);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_DIRECTLY, switchDirect);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _clearEditingItem();
  }

  void _addButton(byte gpio, bool switchDirect, String oName, bool inverted, bool switchApLongPress) {
    _addGpioConfigItem(GPIO_TYPE_BUTTON, gpio, false, false, false, false);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_DIRECTLY, switchDirect);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED, inverted);
    _editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_AP_LONGPRESS, switchApLongPress);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _clearEditingItem();
  }

  byte _getNumberOfChars(byte gType) {
    switch (gType) {
      case GPIO_TYPE_MERGE: return 3;
      case GPIO_TYPE_MODE: return 4;
      case GPIO_TYPE_LED: return 2;
      case GPIO_TYPE_RELAY: return 2;
      case GPIO_TYPE_RGB_LED: return 2;
      case GPIO_TYPE_HTU21D: return 4;
      case GPIO_TYPE_SWITCH: return 1;
      case GPIO_TYPE_BUTTON: return 1;
      default: return 0;
    }
  }

  void _configureTemplate(byte templateIndex) {
    // clear all GPIOs
    for (int i = _numberOfGPIOs->getByte() - 1; i >= 0; i--) {
      _removeGpioConfig(i);
    }
    switch (templateIndex) {
      case 0: {
        // Neo Coolcam
        _addLed(4, false, false, false, false, "led", "", true);
        _addRelay(12, false, true, true, false, "on", "on");
        _addButton(13, true, "on", false, true);
        break;
      }
      case 1: {
        // Milos 1-fach
        _addLed(4, false, false, false, true, "led", "", true);
        _addRelay(13, false, true, true, false, "on", "on");
        _addSwitch(12, true, "on");
        break;
      }
      case 2: {
        // Aubess Touch Switch 2 Gang
        _addLed(0, false, false, false, false, "led", "", true);
        _addMergedProperty(false, true, true, "on", "Channel 1", "");
        _addMergedProperty(false, true, true, "on_2", "Channel 2", "");
        _addRelay(13, true, false, false, false, "on", "");
        _addLed(14, true, false, false, false, "on", "", false);
        _addRelay(4, true, false, false, false, "on_2", "");
        _addLed(1, true, false, false, false, "on_2", "", false);        
        _addButton(3, true, "on", false, false);
        _addButton(5, true, "on_2", false, false);
        break;
      }
      case 3: {
        // Sonoff mini + Switch
        _addLed(13, false, false, false, false, "led", "", true);
        _addRelay(12, false, true, true, false, "on", "on");
        _addButton(0, true, "on", true, true);
        _addSwitch(4, true, "on");
        break;
      }
      case 4: {
        // Sonoff mini + Button
        _addLed(13, false, false, false, false, "led", "", true);
        _addRelay(12, false, true, true, false, "on", "on");
        _addButton(0, true, "on", true, true);
        _addButton(4, true, "on", true, false);
        break;
      }
      case 5: {
        // Sonoff Basic
        _addLed(13, false, false, false, false, "led", "", true);
        _addRelay(12, false, true, true, false, "on", "on");
        _addButton(0, true, "on", false, true);
        break;
      }
      case 6: {
        // Sonoff 4-channel
        _addLed(13, false, false, false, false, "led", "", true);
        _addRelay(12, false, true, true, false, "on", "on");
        _addRelay(5, false, true, true, false, "on_2", "on_2");
        _addRelay(4, false, true, true, false, "on_3", "on_3");
        _addRelay(15, false, true, true, false, "on_4", "on_4");
        _addButton(0, true, "on", false, true);
        _addButton(9, true, "on_2", false, false);
        _addButton(10, true, "on_3", false, false);
        _addButton(14, true, "on_4", false, false);
        break;
      }
      case 7: {
        // Wemos: Relay at D1, Switch at D3
        _addLed(2, false, false, false, false, "led", "", true);
        _addRelay(5, false, true, true, false, "on", "on");
        _addButton(0, 5, "", false, true);
        break;
      }
    }
  }
};

#endif
