#ifndef W_ESP_THING_IO_H
#define W_ESP_THING_IO_H

#include "WDevice.h"
#include "WEspThingIOHtml.h"
#include "WProps.h"
#include "html/WHtmlPages.h"
#include "html/WPage.h"
#include "html/WEspThingPages.h"
#include "hw/W2812.h"
#include "hw/WHtu21D.h"
#include "hw/WPwmDimmer.h"
#include "hw/WRelay.h"
#include "hw/WSht30.h"
#include "hw/WSwitch.h"
#include "WThingGpios.h"

#define DEVICE_ID "switch"


#define CAPTION_EDIT "Edit"
#define CAPTION_REMOVE "Remove"
#define CAPTION_OK "Ok"
#define CAPTION_CANCEL "Cancel"
#define HTTP_USE_TEMPLATE "usetemplate"
#define HTTP_ADD_LED "addled"
#define HTTP_ADD_RELAY "addrelay"
#define HTTP_ADD_DIMMER "adddimmer"
#define HTTP_ADD_RGB_LED "addrgbled"
#define HTTP_ADD_TEMP_SENSOR "addtemp"
#define HTTP_ADD_BUTTON "addbutton"
#define HTTP_ADD_SWITCH "addswitch"
#define HTTP_ADD_MODE "addmode"
#define HTTP_ADD_MERGE "addmerge"
#define HTTP_REMOVE_GPIO "removegpio"

const static char HTTP_BUTTON_VALUE[] PROGMEM = R"=====(
	<div>
  	<form action='/%s' method='POST'>
    	<button class='%s' name='gpio' value='%s'>%s</button>
    </form>
  </div>
)=====";

class WMergedOutput : public WOutput {
 public:
  WMergedOutput(WProperty* merged)
      : WOutput(NO_PIN) {
    _merged = merged;
  }

  void _updateOn() {
    WOutput::_updateOn();
    _merged->asBool(this->isOn());
  };

 private:
  WProperty* _merged;
};

class WEspThingIO : public WDevice {
 public:
  WEspThingIO(WNetwork* network)
      : WDevice(network, DEVICE_ID, DEVICE_ID, DEVICE_TYPE_ON_OFF_SWITCH,  DEVICE_TYPE_LIGHT) {
    Serial.println("WEspThingIO a");
    _gpios = new WThingGpios();
    Serial.println("WEspThingIO b");
    // Configure Device
    _configureDevice();
    Serial.println("WEspThingIO c");
    this->setVisibility(
        ALL);  // this->showAsWebthingDevice->getBoolean() ? ALL : MQTT);
    // HtmlPages
    network->addCustomPage("thathing", [this](){ return new WThingPage(_gpios); }, PSTR("Configure device"));
    network->addCustomPage("json", [this](){ return new WJsonPage(_gpios); }, PSTR("Json"));
    //WPage* configPage = new WPage(network, this->id(), "Configure device");
    //configPage->onPrintPage(std::bind(&WEspThingIO::_printConfigPage, this, std::placeholders::_1));
    //configPage->onSubmitPage(std::bind(&WEspThingIO::_saveConfigPage, this, std::placeholders::_1));
    //network->addCustomPage(configPage);
    // Add LED
    //WPage* utPage = new WPage(network, HTTP_USE_TEMPLATE, "Use a template");
    //utPage->showInMainMenu(false);
    //utPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpUseTemplate, this, std::placeholders::_1));
    //utPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitUseTemplate, this, std::placeholders::_1));
    //utPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(utPage);
    // Add LED
    //WPage* ledPage = new WPage(network, HTTP_ADD_LED, "Add/edit LED");
    //ledPage->showInMainMenu(false);
    //ledPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioLed, this, std::placeholders::_1));
    //ledPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitAddGpioLed, this, std::placeholders::_1));
    //ledPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(ledPage);
    // Add Relay
    //WPage* relayPage = new WPage(network, HTTP_ADD_RELAY, "Add/edit Relay");
    //relayPage->showInMainMenu(false);
    //relayPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioRelay, this, std::placeholders::_1));
    //relayPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitAddGpioRelay, this, std::placeholders::_1));
    //relayPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(relayPage);
    // Add Dimmer
    //WPage* dimmerPage = new WPage(network, HTTP_ADD_DIMMER, "Add/edit Dimmer");
    //dimmerPage->showInMainMenu(false);
    //dimmerPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioDimmer, this, std::placeholders::_1));
    //dimmerPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitAddGpioDimmer, this, std::placeholders::_1));
    //dimmerPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(dimmerPage);
    // Add RGB LED
    //WPage* rgbledPage = new WPage(network, HTTP_ADD_RGB_LED, "Add/edit RGB LED");
    //rgbledPage->showInMainMenu(false);
    //rgbledPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioRgbLed, this, std::placeholders::_1));
    //rgbledPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitAddGpioRgbLed, this, std::placeholders::_1));
    //rgbledPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(rgbledPage);
    // Add Switch
    //WPage* switchPage = new WPage(network, HTTP_ADD_SWITCH, "Add/edit Switch");
    //switchPage->showInMainMenu(false);
    //switchPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioSwitch, this, std::placeholders::_1));
    //switchPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitAddGpioSwitch, this, std::placeholders::_1));
    //switchPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(switchPage);
    // Add Button
    //WPage* buttonPage = new WPage(network, HTTP_ADD_BUTTON, "Add/edit Button");
    //buttonPage->showInMainMenu(false);
    //buttonPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioButton, this, std::placeholders::_1));
    //buttonPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitAddGpioButton, this, std::placeholders::_1));
    //buttonPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(buttonPage);
    // Add Temp sensor
    //WPage* htuPage = new WPage(network, HTTP_ADD_TEMP_SENSOR, "Add/edit temp sensor");
    //htuPage->showInMainMenu(false);
    //htuPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpAddGpioTempSensor, this, std::placeholders::_1));
    //htuPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitAddGpioTempSensor, this, std::placeholders::_1));
    //htuPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(htuPage);
    // Grouped - Mode
    //WPage* groupedPage = new WPage(network, HTTP_ADD_MODE, "Add/edit Mode Property");
    //groupedPage->showInMainMenu(false);
    //groupedPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpAddModeProperty, this, std::placeholders::_1));
    //groupedPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitAddModeProperty, this, std::placeholders::_1));
    //groupedPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(groupedPage);
    // Grouped - Merge
    //groupedPage = new WPage(network, HTTP_ADD_MERGE, "Add/edit Merged Property");
    //groupedPage->showInMainMenu(false);
    //groupedPage->onPrintPage(std::bind(&WEspThingIO::_handleHttpAddMergeProperty, this, std::placeholders::_1));
    //groupedPage->onSubmitPage(std::bind(&WEspThingIO::_handleHttpSubmitAddMergeProperty, this, std::placeholders::_1));
    //groupedPage->targetAfterSubmitting(configPage);
    //network->addCustomPage(groupedPage);
    // Remove GPIO
    //WPage* removeGpio = new WPage(network, HTTP_REMOVE_GPIO, "Remove GPIO");
    //removeGpio->showInMainMenu(false);
    //removeGpio->onPrintPage(std::bind(&WEspThingIO::_handleHttpRemoveGpioButton, this, std::placeholders::_1));
    //network->addCustomPage(removeGpio);
  }

  /*void _handleHttpUseTemplate(WPage* page) {
    page->configPageBegin(HTTP_USE_TEMPLATE);
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

  void _handleHttpAddGpioLed(WPage* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_LED);
    _updateEditingItem(request, page);
    // GPIO
    addGpioChooser(page, true, PSTR("GPIO:"), "gp", BYTE_GPIO, 13);
    byte aProps = (_editingItem != nullptr ? _editingItem->byteArrayValue(BYTE_CONFIG) : 0b00000000);
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

  void _handleHttpAddGpioRelay(WPage* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_RELAY);
    _updateEditingItem(request, page);
    // GPIO
    addGpioChooser(page, true, PSTR("GPIO:"), "gp", BYTE_GPIO, 13);
    byte aProps = (_editingItem != nullptr
                       ? _editingItem->byteArrayValue(BYTE_CONFIG)
                       : 0b00000110);
    _printVisibility(page, aProps, GPIO_TYPE_RELAY, "on", false);
    // Inverted
    // WHtmlPages::checkBox(page, "iv", "inverted");
    page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv",
                 (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""),
                 "", "Inverted");
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioDimmer(WPage* page) {




    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_DIMMER);
    bool newItem = _updateEditingItem(request, page);
    byte aProps = (newItem ? 0 : _editingItem->byteArrayValue(BYTE_CONFIG));


    addGpioChooser(page, true, PSTR("GPIO:"), "gp", BYTE_GPIO, 13);
    _handleHttpAddDualProperty(request, page, true, PSTR("On"), PSTR("Level"));
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioRgbLed(WPage* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_RGB_LED);
    bool newItem = _updateEditingItem(request, page);
    // GPIO
    addGpioChooser(page, true, PSTR("GPIO:"), "gp", BYTE_GPIO, 13);
    // Number of LEDs
    String noLeds((byte)(newItem ? 1 : _editingItem->byteArrayValue(BYTE_NO_OF_LEDS)));
    WHtml::textField(page, "nol", "Number of LEDs", 3, noLeds.c_str());
    // Properties
    byte aProps = (newItem ? 0b00000110 : _editingItem->byteArrayValue(BYTE_CONFIG));
    _printVisibility(page, aProps, GPIO_TYPE_RGB_LED, "on", true);
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioTempSensor(WPage* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_TEMP_SENSOR);
    bool newItem = _updateEditingItem(request, page);
    byte aProps = (newItem ? 0 : _editingItem->byteArrayValue(BYTE_CONFIG));
    bool isHtu21 = (newItem ? false : bitRead(aProps, BIT_CONFIG_TEMP_SENSOR_TYPE));
    page->printf(HTTP_COMBOBOX_BEGIN, "Model:", "st");
    page->printf(HTTP_COMBOBOX_ITEM, "0", (!isHtu21 ? HTTP_SELECTED : ""), F("SHT30"));
    page->printf(HTTP_COMBOBOX_ITEM, "1", (isHtu21 ? HTTP_SELECTED : ""), F("HTU21"));
    page->print(FPSTR(HTTP_COMBOBOX_END));

    addGpioChooser(page, false, PSTR("SDA:"), "sda", BYTE_GPIO, 4);
    addGpioChooser(page, false, PSTR("SCL:"), "scl", BYTE_SCL, 5);
    _handleHttpAddDualProperty(request, page, true, PSTR("Temperature"), PSTR("Humidity"));
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioSwitch(WPage* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_SWITCH);
    bool newItem = _updateEditingItem(request, page);
    byte aProps =
        (newItem ? 0 : _editingItem->byteArrayValue(BYTE_CONFIG));
    _handleHttpAddGpioSwitchButton(request, page, newItem, aProps);
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddGpioButton(WPage* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_BUTTON);
    bool newItem = _updateEditingItem(request, page);
    byte aProps = (newItem ? 0 : _editingItem->byteArrayValue(BYTE_CONFIG));
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
    byte aProps = (newItem ? 0 : _editingItem->byteArrayValue(BYTE_CONFIG));
    bool wt = (newItem ? true : bitRead(aProps, BIT_CONFIG_PROPERTY_WEBTHING));
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "wa", (wt ? HTTP_BLOCK : HTTP_NONE), "wb", HTTP_NONE);
    String c = F(" name:");
    String fn = firstTitle;
    fn.toLowerCase();
    String fnc = firstTitle;
    fnc.concat(c);
    String sn = secondTitle;
    sn.toLowerCase();
    String snc = secondTitle;
    snc.concat(c);
    // Property name
    char* gName = _getSubString(_editingItem, FIRST_NAME);
    WHtml::textField(
        page, "fn", fnc.c_str(), 8,
        (gName != nullptr ? gName : _getNextName(GPIO_TYPE_MODE, FIRST_NAME, fn).c_str()));
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
    fnc = firstTitle;
    fnc.concat(c);
    snc = secondTitle;
    snc.concat(c);
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

  void _handleHttpAddModeProperty(WPage* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_MODE);
    _handleHttpAddDualProperty(request, page, true, PSTR("Switch"), PSTR("Mode"));
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, VALUE_GET, CAPTION_CANCEL);
  }

  void _handleHttpAddMergeProperty(WPage* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_MERGE);
    bool newItem = _updateEditingItem(request, page);
    byte aProps = (newItem ? 0b00000111 : _editingItem->byteArrayValue(BYTE_CONFIG));
    _printVisibility(page, aProps, GPIO_TYPE_MERGE, "merge", false);
    page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv",
                 (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""),
                 "", "Inverted");
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

  void _handleHttpSubmitUseTemplate(AsyncWebServerRequest* request) {
    _configureTemplate(request->arg("dt").toInt());
  }

  void _handleHttpSubmitAddGpioLed(AsyncWebServerRequest* request) {
    String voo = request->arg(HTTP_PROPERTY_VISIBILITY);
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));

    _addLed(request->arg("gp").toInt(), gr, mq, wt,
            (request->arg("iv") == HTTP_TRUE), (!gr ? request->arg("pn") : voo),
            (!gr ? request->arg("wtt") : request->arg("mot")),
            (request->arg("ls") == HTTP_TRUE));
  }

  void _handleHttpSubmitAddGpioRelay(AsyncWebServerRequest* request) {
    String voo = request->arg(HTTP_PROPERTY_VISIBILITY);
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));
    _addRelay(request->arg("gp").toInt(), gr, mq, wt,
              (request->arg("iv") == HTTP_TRUE),
              (!gr ? request->arg("pn") : voo),
              (!gr ? request->arg("wtt") : request->arg("mot")));
  }

  void _handleHttpSubmitAddGpioDimmer(AsyncWebServerRequest* request) {
    _addDimmer(request->arg("gp").toInt(),
               (request->arg("wt") == HTTP_TRUE),
               request->arg("fn"), request->arg("ft"),
               request->arg("sn"), request->arg("st"));
  }

  void _handleHttpSubmitAddGpioRgbLed(AsyncWebServerRequest* request) {
    String voo = request->arg(HTTP_PROPERTY_VISIBILITY);
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));
    _addRgbLed(request->arg("gp").toInt(), gr, mq, wt,
               request->arg("nol").toInt(), (!gr ? request->arg("pn") : voo),
               (!gr ? request->arg("wtt") : request->arg("mot")),
               (request->arg("gim") == HTTP_TRUE));
  }

  void _handleHttpSubmitAddGpioTempSensor(AsyncWebServerRequest* request) {
    _addTempSensor(request->arg("sda").toInt(), request->arg("scl").toInt(),
                   true, (request->arg("wt") == HTTP_TRUE),
                   request->arg("st").toInt(),
                   request->arg("fn"), request->arg("ft"),
                   request->arg("sn"), request->arg("st"));
  }

  void _handleHttpSubmitAddModeProperty(AsyncWebServerRequest* request) {
    _addGroupedProperty((request->arg("wt") == HTTP_TRUE),
                        request->arg("fn"), request->arg("ft"),
                        request->arg("sn"), request->arg("st"));
  }

  void _handleHttpSubmitAddMergeProperty(AsyncWebServerRequest* request) {
    String voo = request->arg(HTTP_PROPERTY_VISIBILITY);
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));
    _addMergedProperty(gr, mq, wt,
                       (request->arg("iv") == HTTP_TRUE),
                       request->arg("pn"),
                       (!gr ? request->arg("wtt") : request->arg("mot")),
                       (gr ? voo : ""));
  }

  void _handleHttpSubmitAddGpioSwitch(AsyncWebServerRequest* request) {
    bool switchDirect = (request->arg("mq") == HTTP_TRUE);
    _addSwitch(request->arg("gp").toInt(), switchDirect,
               (switchDirect ? request->arg("lgp") : request->arg("pn")));
  }

  void _handleHttpSubmitAddGpioButton(AsyncWebServerRequest* request) {
    bool switchDirect = (request->arg("mq") == HTTP_TRUE);
    _addButton(request->arg("gp").toInt(), switchDirect,
               (switchDirect ? request->arg("lgp") : request->arg("pn")),
               (request->arg("iv") == HTTP_TRUE),
               (request->arg("lp") == HTTP_TRUE));
  }

  void _handleHttpRemoveGpioButton(WPage* page) {
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

  void _saveConfigPage(AsyncWebServerRequest* request) {}*/

 protected:
  
  /*
  bool _addGpioChooserItem(Print* page, byte gpio, const char* title, byte bytePos, byte defaultGpio) {
    int aGpio = (_editingItem != nullptr ? _editingItem->byteArrayValue(bytePos) : defaultGpio);
    char* pNumber = new char[2];
    sprintf(pNumber, "%d", gpio);
    if (((_editingItem != nullptr) && (aGpio == gpio)) || (_isGpioFree(gpio)))
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
    /// 16, 3, 1, 10, 9 This may be problematic if you have relays or other peripherals connected to those GPIOs. The following GPIOs output a HIGH signal on boot
    // _addGpioChooserItem(page, 9, "9", bytePos, defaultGpio);
    // _addGpioChooserItem(page, 10, "10", bytePos, defaultGpio);
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
      case GPIO_TYPE_DIMMER:
        return "Dimmer";
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
      case GPIO_TYPE_TEMP_SENSOR:
        return "TempSens";
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
      case GPIO_TYPE_DIMMER:
        return HTTP_ADD_DIMMER;
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
      case GPIO_TYPE_TEMP_SENSOR:
        return HTTP_ADD_TEMP_SENSOR;
      default:
        return HTTP_ADD_LED;
    }
  }*/

  /*void _addLinkedGpioChooser(Print* page, boolean newItem) {
    // BYTE_SWITCH_LINKED_GPIO
    page->printf(HTTP_COMBOBOX_BEGIN, "Select output GPIO:", "lgp");
    WProperty* linkedName = (newItem ? nullptr : _getSubProperty(_editingItem, FIRST_NAME));
    // page->printf(HTTP_COMBOBOX_ITEM, "255", (BYTE_NOT_LINKED_GPIO ==
    // linkedGpio ? HTTP_SELECTED : ""), "None");
    for (byte i = 0; i < _numberOfGPIOs->asByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      byte gGpio = gConfig->byteArrayValue(BYTE_GPIO);
      char* gName = _getSubString(gConfig, FIRST_NAME);
      byte aProps = gConfig->byteArrayValue(BYTE_CONFIG);
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
  }*/

  /*void _printVisibility(Print* page, byte aProps, byte aType, String baseName, bool groupedInModeCheckBox) {
    bool gr = bitRead(aProps, BIT_CONFIG_PROPERTY_GROUPED);
    bool mq = bitRead(aProps, BIT_CONFIG_PROPERTY_MQTT);
    bool wt = bitRead(aProps, BIT_CONFIG_PROPERTY_WEBTHING);
    char* eName = _getSubString(_editingItem, FIRST_NAME);
    if (aType == GPIO_TYPE_MERGE) {
      WHtml::textField(page, "pn", "Property name:", 8, (eName != nullptr ? eName : _getNextName(aType, FIRST_NAME, baseName).c_str()));
      eName = _getSubString(_editingItem, SECOND_NAME);
    }
    // ComboBox
    page->printf(HTTP_COMBOBOX_OPTION_BEGIN, "Property:", HTTP_PROPERTY_VISIBILITY);
    // MQTT and Webthings
    WHtml::comboBoxItem(page, "MQTT and Webthings", "2", !gr && mq && wt);
    // MQTT
    WHtml::comboBoxItem(page, "Only MQTT", "1", !gr && mq && !wt);
    // None
    if (aType != GPIO_TYPE_MERGE) {
      WHtml::comboBoxItem(page, "No property", "0", !gr && !mq && !wt);
    }
    // List grouped properties
    for (byte i = 0; i < _numberOfGPIOs->asByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
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
      // BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE
      bool gim = bitRead(aProps, BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE);
      WHtml::checkBox(page, "gim", PSTR("Add different modes in dropdown list"), gim);
    }
    page->print(FPSTR(HTTP_DIV_END));
  }*/

 private:
  WThingGpios* _gpios;

  void _configureOutput(WOutput* output, byte gConfigIndex, WValue* gConfig, bool gr, bool mq, bool wt) {
    /*this->addOutput(output);
    char* gName = _gpios->getSubString(gConfigIndex, FIRST_NAME);
    char* gTitle = _gpios->getSubString(gConfigIndex, FIRST_TITLE);
    if (gr) {
      WValue* groupedGpio = _gpios->getGroupedGpioByName(gName);
      if (groupedGpio != nullptr) {
        if (groupedGpio->byteArrayValue(BYTE_TYPE) == GPIO_TYPE_MODE) {
          char* modeName = _gpios->getSubString(groupedGpio, SECOND_NAME);
          if (modeName != nullptr) {
            WProperty* modeProp = this->getPropertyById(modeName);
            if (modeProp != nullptr) {
              byte gType = gConfig->byteArrayValue(BYTE_TYPE);
              bool pim = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE);
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
                  modeProp->value()->asString(gTitle);
                }
                _configureOutputModes(output, gConfig, modeProp->isVisible(MQTT), modeProp->isVisible(WEBTHING));
              }
            }
          }
        }
        //tbi
        //output->on(groupedGpio);
        output->setId(gTitle);
        network()->debug(F("grouped %s has added an output"), groupedGpio->title(), output->id());
      } else {
        network()->error("Can't find grouped property. Name of grouped property: '%s'", gName);
      }
    } else if ((mq) || (wt)) {
      WProperty* relayProp = WProps::createOnOffProperty(gTitle);
      relayProp->visibilityMqtt(mq);
      relayProp->visibilityWebthing(wt);
      output->on(relayProp);
      this->addProperty(relayProp, gName);
      _configureOutputModes(output, gConfig, mq, wt);
    }*/
  }

  void _configureOutputModes(WOutput* output, WValue* gConfig, bool mq, bool wt) {
    /*if (output->countModes() > 1) {
      // Mode property
      String mName = String(_gpios->getSubString(gConfig, FIRST_NAME));
      mName.concat(F("_mode"));
      String mTitle = String(_gpios->getSubString(gConfig, FIRST_TITLE));
      mTitle.concat(F(" Mode"));
      WProperty* modeProp = WProps::createStringProperty(mTitle.c_str());
      SETTINGS->add(modeProp->value(), mName.c_str());
      this->addProperty(modeProp, mName.c_str());
      modeProp->visibility(mq, wt);
      // Enums
      for (byte i = 0; i < output->countModes(); i++) {
        modeProp->addEnumString(output->modeTitle(i));
      }
      // Default/Initial value
      if (!modeProp->value()->equalsString("")) {
        output->setModeByTitle(modeProp->value()->c_str());
      } else {
        modeProp->value()->asString(output->modeTitle(output->mode()));
      }
      // Change notification
      modeProp->addListener([this, modeProp, output]() {
        output->setModeByTitle(modeProp->value()->c_str());
      });
    }*/
  }

  void _notifyGroupedChange(WProperty* property, byte subStringIndex) {
    /*WProperty* groupedGpio = _gpios->getGroupedGpioBySubString(property->title(), subStringIndex);
    if (groupedGpio != nullptr) {
      char* gName = _gpios->getSubString(groupedGpio, FIRST_NAME);
      char* mName = _gpios->getSubString(groupedGpio, SECOND_NAME);
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
                  output->setOn((onOffProp->asBool()) && (output->equalsId(outputName.c_str())));
                });
          } else {
            network()->error(F("Grouped GPIO '%s' has no outputs"), groupedGpio->id());
          }
          network()->error(F("Grouped GPIO '%s' has no outputs (tbi)"), groupedGpio->title());
        } else {
          network()->error(F("Name property or mode property is missing for grouped property: '%s'"), property->value()->c_str());
        }
      } else {
        network()->error(F("Name or mode name is missing for grouped property: '%s'"), property->value()->c_str());
      }
    } else {
      network()->error(F("Can't find grouped property. Name of grouped property: '%s'"), property->value()->c_str());
    }*/
  }

  void _notifyMergedChange(WProperty* property) {
    /*WProperty* mergedGpio = _gpios->getGroupedGpioBySubString(property->title(), FIRST_NAME);
    if (mergedGpio != nullptr) {
      char* gName = _gpios->getSubString(mergedGpio, FIRST_NAME);
      bool iv = mergedGpio->value()->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED);
      if (mergedGpio->hasOutputs()) {
        mergedGpio->outputs()->forEach([this, property, iv](WOutput* output) {
          output->setOn(iv ? !property->asBool() : property->asBool());
        });
      }
      network()->error(F("Merged GPIO '%s' has no outputs (tbi)"), mergedGpio->title());
    } else {
      network()->error(F("Can't find merged property. Name of merged property: '%s'"), property->value()->c_str());
    }*/
  }

  void _configureDevice() {
    Serial.println("_configureDevice a");
    // 1. Grouped
    for (byte i = 0; i < _gpios->numberOfGPIOs()->asByte(); i++) {
      Serial.println("_configureDevice a.");
      WValue* gConfig = _gpios->getGpioConfig(i);
      Serial.println("_configureDevice a..");
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      LOG->debug("ich habe hier %d", gType);
      if (gType == GPIO_TYPE_MODE) {
        const char* gName = _gpios->getSubString(i, FIRST_NAME);
        const char* gTitle = _gpios->getSubString(i, FIRST_TITLE);
        const char* mName = _gpios->getSubString(i, SECOND_NAME);        
        const char* mTitle = _gpios->getSubString(i, SECOND_TITLE);
        bool mq = true;
        bool wt = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
        WProperty* onOffProp = WProps::createOnOffProperty(gTitle);
        onOffProp->visibilityMqtt(mq);
        onOffProp->visibilityWebthing(wt);
        onOffProp->addListener([this, onOffProp]() { _notifyGroupedChange(onOffProp, FIRST_NAME); });
        this->addProperty(onOffProp, gName);        
        WProperty* modeProp = WProps::createStringProperty(mTitle);
        //tbi SETTINGS->add(modeProp->value(), mName);
        modeProp->visibilityMqtt(mq);
        modeProp->visibilityWebthing(wt);
        modeProp->addListener([this, modeProp]() {
          _notifyGroupedChange(modeProp, SECOND_NAME);
        });
        this->addProperty(modeProp, mName);
      }
    }
    Serial.println("_configureDevice b");
    // 2. Merged
    for (byte i = 0; i < _gpios->numberOfGPIOs()->asByte(); i++) {
      WValue* gConfig = _gpios->getGpioConfig(i);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      if (gType == GPIO_TYPE_MERGE) {
        const char* gName = _gpios->getSubString(i, FIRST_NAME);
        const char* gTitle = _gpios->getSubString(i, FIRST_TITLE);
        network()->debug(F("add merge item '%s'"), gName);
        bool mq = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT);
        bool wt = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
        bool gr = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_GROUPED);
        bool iv = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED);
        WProperty* onOffProp = WProps::createOnOffProperty(gTitle);
        onOffProp->visibilityMqtt(mq);
        onOffProp->visibilityWebthing(wt);
        onOffProp->asBool(iv);
        onOffProp->addListener([this, onOffProp]() { _notifyMergedChange(onOffProp); });
        this->addProperty(onOffProp, gName);
        if (gr) {
          const char* pgName = _gpios->getSubString(i, SECOND_NAME);
          WProperty* groupedGpio = this->getPropertyById(pgName);
          if (groupedGpio != nullptr) {
            if (groupedGpio->byteArrayValue(BYTE_TYPE) == GPIO_TYPE_MODE) {
              const char* modeName = _gpios->getSubString(i, SECOND_NAME);
              if (modeName != nullptr) {
                WProperty* modeProp = this->getPropertyById(modeName);
                if (modeProp != nullptr) {
                  modeProp->addEnumString(gTitle);
                  if (modeProp->isNull()) {
                    modeProp->asString(gTitle);
                  }
                }
              }
            }
            WMergedOutput* output = new WMergedOutput(onOffProp);
            output->on(groupedGpio);
            output->setId(gTitle);
            network()->debug("grouped %s has added a merged output", groupedGpio->title(), output->id());
          } else {
            network()->error("Can't find grouped property. Name of grouped property: '%s'", pgName);
          }
        }
      }
    }
    Serial.println("_configureDevice c");
    // 3. Outputs
    for (byte i = 0; i < _gpios->numberOfGPIOs()->asByte(); i++) {
      WValue* gConfig = _gpios->getGpioConfig(i);
      bool gr = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_GROUPED);
      bool mq = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT);
      bool wt = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
      switch (gConfig->byteArrayValue(BYTE_TYPE)) {
        case GPIO_TYPE_LED: {
          network()->debug(F("add led"));
          WLed* led = new WLed(gConfig->byteArrayValue(BYTE_GPIO));
          _configureOutput(led, i, gConfig, gr, mq, wt);
          // Inverted
          led->setInverted(gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
          // Linkstate
          if (gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_LINKSTATE)) {
            network()->setStatusLed(led, false);
          }
          break;
        }
        case GPIO_TYPE_RELAY: {
          network()->debug(F("add relay"));
          WRelay* relay = new WRelay(gConfig->byteArrayValue(BYTE_GPIO), gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
          _configureOutput(relay, i, gConfig, gr, mq, wt);
          break;
        }
        case GPIO_TYPE_DIMMER: {
          network()->debug(F("add dimmer"));
          WPwmDimmer* pwm = new WPwmDimmer(gConfig->byteArrayValue(BYTE_GPIO));
          _configureOutput(pwm, i, gConfig, gr, mq, wt);
          // Level
          const char* gName = _gpios->getSubString(i, SECOND_NAME);
          const char* gTitle = _gpios->getSubString(i, SECOND_TITLE);
          WProperty* level = WProps::createBrightnessProperty(gTitle);
          //tbi
          //SETTINGS->add(level->value(), gTitle);
          level->visibility(true, true);
          this->addProperty(level, gName);
          pwm->level(level);
          // WProps::createBrightnessProperty(_levId.c_str(), _levTitle.c_str());
          break;
        }
        case GPIO_TYPE_RGB_LED: {
          network()->debug(F("add RGB led"));
          W2812Led* ledStrip = new W2812Led(network(), gConfig->byteArrayValue(BYTE_GPIO), gConfig->byteArrayValue(BYTE_NO_OF_LEDS));
          _configureOutput(ledStrip, i, gConfig, gr, mq, wt);
          // ledStrip->led
          if (!gr) {
          }
          break;
        }
      }
    }
    Serial.println("_configureDevice d");
    // 4. Inputs
    for (byte i = 0; i < _gpios->numberOfGPIOs()->asByte(); i++) {
      WValue* gConfig = _gpios->getGpioConfig(i);
      const char* gName = _gpios->getSubString(i, FIRST_NAME);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      switch (gType) {
        case GPIO_TYPE_BUTTON:
        case GPIO_TYPE_SWITCH: {
          network()->debug(F("add button/switch '%s'"), gName);
          bool inverted = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED);
          bool longPress = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_AP_LONGPRESS);
          byte bMode = (gType == GPIO_TYPE_SWITCH ? MODE_SWITCH : (longPress ? MODE_BUTTON_LONG_PRESS : MODE_BUTTON));
          WSwitch* button = new WSwitch(gConfig->byteArrayValue(BYTE_GPIO), bMode, inverted);
          if (longPress) {
            button->setOnLongPressed([this]() {
              if (!network()->isSoftAP()) {
                SETTINGS->forceAPNextStart();
                delay(200);
                ESP.restart();
              }
            });
          }
          this->addInput(button);
          bool switchDirect = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_DIRECTLY);
          if (switchDirect) {
            button->setProperty(this->getPropertyById(gName));
          } else if ((gName != nullptr) && (strcmp(gName, "") != 0)) {
            // Only trigger via MQTT
            WProperty* triggerProperty = WProps::createOnOffProperty(gName);
            triggerProperty->visibility(MQTT);
            this->addProperty(triggerProperty, gName);
            button->setTriggerProperty(triggerProperty);
          }
          break;
        }
        case GPIO_TYPE_TEMP_SENSOR: {
          network()->debug(F("add temperature sensor '%s'"), gName);
          // Sensor
          WI2CTemperature* htu = nullptr;
          bool isHtu21 = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_TEMP_SENSOR_TYPE);
          if (isHtu21) {
            htu = new WHtu21D(gConfig->byteArrayValue(BYTE_GPIO), gConfig->byteArrayValue(BYTE_SCL));
          } else {
            htu = new WSht30(gConfig->byteArrayValue(BYTE_GPIO), gConfig->byteArrayValue(BYTE_SCL));
          }
          this->addInput(htu);
          // Properties
          bool mq = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT);
          bool wt = gConfig->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
          const char* gTitle = _gpios->getSubString(i, FIRST_TITLE);
          WProperty* temperature = WProps::createTemperatureProperty(gTitle);
          temperature->visibility(mq, wt);
          this->addProperty(temperature, gName);
          gName = _gpios->getSubString(i, SECOND_NAME);
          gTitle = _gpios->getSubString(i, SECOND_TITLE);
          WProperty* humidity = WProps::createHumidityProperty(gTitle);
          this->addProperty(humidity, gName);
          // Set properties to sensor
          htu->setProperty(temperature);
          htu->setHumidity(humidity);
          break;
        }
      }
    }
    Serial.println("_configureDevice e");
  }

  
  /*void _configureTemplate(byte templateIndex) {
    // clear all GPIOs
    for (int i = _numberOfGPIOs->asByte() - 1; i >= 0; i--) {
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
        _addMergedProperty(false, true, true, false, "on", "Channel 1", "");
        _addMergedProperty(false, true, true, false, "on_2", "Channel 2", "");
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
  }*/
};

#endif
