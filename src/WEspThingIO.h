#ifndef W_ESP_THING_IO_H
#define W_ESP_THING_IO_H

#include "WDevice.h"
#include "WHtmlPages.h"
#include "WEspThingIOHtml.h"
#include "WPage.h"
#include "WProperty.h"
#include "WRelay.h"
#include "WSwitch.h"

#define DEVICE_ID "switch"
#define MAX_GPIOS 12
#define MAX_PROP_BYTES 4
#define GPIO_TYPE_LED 0
#define GPIO_TYPE_RELAY 1
#define GPIO_TYPE_BUTTON 2
#define GPIO_TYPE_SWITCH 3
#define GPIO_TYPE_GROUPED 4

#define BYTE_TYPE 0
#define BYTE_GPIO 1
#define BYTE_CONFIG 2
#define BYTE_LINKED_GPIO 3
#define BYTE_NOT_LINKED_GPIO 0xFF
#define NO_INDEX 0xFF

#define BIT_CONFIG_PROPERTY_GROUPED 0
#define BIT_CONFIG_PROPERTY_MQTT 1
#define BIT_CONFIG_PROPERTY_WEBTHING 2
#define BIT_CONFIG_LED_INVERTED 3
#define BIT_CONFIG_LED_LINKSTATE 4
#define BIT_CONFIG_SWITCH_AP_LONGPRESS 4

#define CAPTION_EDIT "Edit"
#define CAPTION_REMOVE "Remove"
#define CAPTION_OK "Ok"
#define CAPTION_CANCEL "Cancel"
#define HTTP_USE_TEMPLATE "usetemplate"
#define HTTP_ADD_LED "addled"
#define HTTP_ADD_RELAY "addrelay"
#define HTTP_ADD_BUTTON "addbutton"
#define HTTP_ADD_SWITCH "addswitch"
#define HTTP_ADD_GROUPED "addgrouped"
#define HTTP_REMOVE_GPIO "removegpio"

const byte* DEFAULT_PROP_ARRAY = (const byte[]){0, 0, 0, 0};
const static char HTTP_BUTTON_VALUE[] PROGMEM = R"=====(
	<div>
  	<form action='/%s' method='POST'>
    	<button class='%s' name='gpio' value='%s'>%s</button>
    </form>
  </div>
)=====";

class WEspThingIO : public WDevice {
 public:
  WEspThingIO(WNetwork* network)
      : WDevice(network, DEVICE_ID, DEVICE_ID, DEVICE_TYPE_ON_OFF_SWITCH,
                DEVICE_TYPE_LIGHT) {
    this->editingItem = nullptr;
    this->editingIndex = NO_INDEX;
    this->numberOfGPIOs =
        network->getSettings()->setByte("numberOfGPIOs", 0, MAX_GPIOS);
    this->numberOfGPIOs->setVisibility(NONE);
    this->addProperty(this->numberOfGPIOs);
    byte nog = this->numberOfGPIOs->getByte();
    this->numberOfGPIOs->setByte(0);
    for (byte i = 0; i < nog; i++) {
      network->debug("add %d", i);
      this->addGpioConfig();
      this->numberOfGPIOs->setByte(i + 1);
      network->debug("added %d", i);
    }
    // Configure Device
    network->debug("a");
    configureDevice();
    network->debug("b");
    this->setVisibility(
        ALL);  // this->showAsWebthingDevice->getBoolean() ? ALL : MQTT);
    // HtmlPages
    WPage* configPage = new WPage(this->getId(), "Configure device");
    configPage->setPrintPage(std::bind(&WEspThingIO::printConfigPage, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2));
    configPage->setSubmittedPage(std::bind(&WEspThingIO::saveConfigPage, this,
                                           std::placeholders::_1,
                                           std::placeholders::_2));
    network->addCustomPage(configPage);
    // Add LED
    WPage* utPage = new WPage(HTTP_USE_TEMPLATE, "Use a template");
    utPage->setShowInMainMenu(false);
    utPage->setPrintPage(std::bind(&WEspThingIO::handleHttpUseTemplate, this,
                                   std::placeholders::_1,
                                   std::placeholders::_2));
    utPage->setSubmittedPage(
        std::bind(&WEspThingIO::handleHttpSubmitUseTemplate, this,
                  std::placeholders::_1, std::placeholders::_2));
    utPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(utPage);
    // Add LED
    WPage* ledPage = new WPage(HTTP_ADD_LED, "Add/edit LED");
    ledPage->setShowInMainMenu(false);
    ledPage->setPrintPage(std::bind(&WEspThingIO::handleHttpAddGpioLed, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
    ledPage->setSubmittedPage(
        std::bind(&WEspThingIO::handleHttpSubmitAddGpioLed, this,
                  std::placeholders::_1, std::placeholders::_2));
    ledPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(ledPage);
    // Add Relay
    WPage* relayPage = new WPage(HTTP_ADD_RELAY, "Add/edit Relay");
    relayPage->setShowInMainMenu(false);
    relayPage->setPrintPage(std::bind(&WEspThingIO::handleHttpAddGpioRelay,
                                      this, std::placeholders::_1,
                                      std::placeholders::_2));
    relayPage->setSubmittedPage(
        std::bind(&WEspThingIO::handleHttpSubmitAddGpioRelay, this,
                  std::placeholders::_1, std::placeholders::_2));
    relayPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(relayPage);
    // Add Switch
    WPage* switchPage = new WPage(HTTP_ADD_SWITCH, "Add/edit Switch");
    switchPage->setShowInMainMenu(false);
    switchPage->setPrintPage(std::bind(&WEspThingIO::handleHttpAddGpioSwitch,
                                       this, std::placeholders::_1,
                                       std::placeholders::_2));
    switchPage->setSubmittedPage(
        std::bind(&WEspThingIO::handleHttpSubmitAddGpioSwitch, this,
                  std::placeholders::_1, std::placeholders::_2));
    switchPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(switchPage);
    // Add Button
    WPage* buttonPage = new WPage(HTTP_ADD_BUTTON, "Add/edit Button");
    buttonPage->setShowInMainMenu(false);
    buttonPage->setPrintPage(std::bind(&WEspThingIO::handleHttpAddGpioButton,
                                       this, std::placeholders::_1,
                                       std::placeholders::_2));
    buttonPage->setSubmittedPage(
        std::bind(&WEspThingIO::handleHttpSubmitAddGpioButton, this,
                  std::placeholders::_1, std::placeholders::_2));
    buttonPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(buttonPage);
    // Add grouped property
    WPage* groupedPage =
        new WPage(HTTP_ADD_GROUPED, "Add/edit Grouped Property");
    groupedPage->setShowInMainMenu(false);
    groupedPage->setPrintPage(std::bind(&WEspThingIO::handleHttpAddOnOffMode,
                                        this, std::placeholders::_1,
                                        std::placeholders::_2));
    groupedPage->setSubmittedPage(
        std::bind(&WEspThingIO::handleHttpSubmitAddOnOffMode, this,
                  std::placeholders::_1, std::placeholders::_2));
    groupedPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(groupedPage);
    // Remove GPIO
    WPage* removeGpio = new WPage(HTTP_REMOVE_GPIO, "Remove GPIO");
    removeGpio->setShowInMainMenu(false);
    removeGpio->setPrintPage(std::bind(&WEspThingIO::handleHttpRemoveGpioButton,
                                       this, std::placeholders::_1,
                                       std::placeholders::_2));
    // ledPage->setSubmittedPage(std::bind(&WEspThingIO::submittedAddLedPage,
    // this, std::placeholders::_1, std::placeholders::_2));
    network->addCustomPage(removeGpio);
  }

  void handleHttpUseTemplate(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_USE_TEMPLATE);
    // deviceType
    page->printf(HTTP_COMBOBOX_BEGIN, "Model:", "dt");
    page->printf(HTTP_COMBOBOX_ITEM, "0", HTTP_SELECTED, F("Neo Coolcam"));
    page->printf(HTTP_COMBOBOX_ITEM, "1", "", F("Milos Single Switch"));
    page->printf(HTTP_COMBOBOX_ITEM, "2", "", F("Milos Double Switch"));
    page->printf(HTTP_COMBOBOX_ITEM, "3", "", F("Sonoff Mini + Switch"));
    page->printf(HTTP_COMBOBOX_ITEM, "4", "", F("Sonoff Mini + Button"));
    page->printf(HTTP_COMBOBOX_ITEM, "5", "", F("Sonoff Basic"));
    page->printf(HTTP_COMBOBOX_ITEM, "6", "", F("Sonoff 4-channel"));
    page->printf(HTTP_COMBOBOX_ITEM, "7", "",
                 F("Wemos: Relay at D1, Switch at D3"));
    page->print(FPSTR(HTTP_COMBOBOX_END));
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
  }

  void handleHttpAddGpioLed(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_LED);
    updateEditingItem(request, page);
    // GPIO
    addGpioChooser(page, true);
    byte aProps = (this->editingItem != nullptr
                       ? this->editingItem->getByteArrayValue(BYTE_CONFIG)
                       : 0b00000000);
    printVisibility(page, aProps, GPIO_TYPE_LED, "led");
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
    page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
  }

  void handleHttpAddGpioRelay(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_RELAY);
    updateEditingItem(request, page);
    // GPIO
    addGpioChooser(page, true);
    byte aProps = (this->editingItem != nullptr
                       ? this->editingItem->getByteArrayValue(BYTE_CONFIG)
                       : 0b00000110);
    printVisibility(page, aProps, GPIO_TYPE_RELAY, "on");
    // Inverted
    page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv",
                 (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""),
                 "", "Inverted");
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
  }

  void handleHttpAddGpioSwitch(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_SWITCH);
    handleHttpAddGpioSwitchButton(request, page);
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
  }

  void handleHttpAddGpioButton(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_BUTTON);
    handleHttpAddGpioSwitchButton(request, page);
    byte aProps = (this->editingItem != nullptr
                       ? this->editingItem->getByteArrayValue(BYTE_CONFIG)
                       : 0);
    // Inverted
    page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv",
                 (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""),
                 "", "Inverted");
    page->printf(
        HTTP_CHECKBOX_OPTION, "lp", "lp",
        (bitRead(aProps, BIT_CONFIG_SWITCH_AP_LONGPRESS) ? HTTP_CHECKED : ""),
        "", "Open AccesPoint on long press");
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
  }

  void handleHttpAddOnOffMode(AsyncWebServerRequest* request, Print* page) {
    page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_GROUPED);
    updateEditingItem(request, page);
    byte aProps = (this->editingItem != nullptr
                       ? this->editingItem->getByteArrayValue(BYTE_CONFIG)
                       : 0);
    bool wt = bitRead(aProps, BIT_CONFIG_PROPERTY_WEBTHING);
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "wa", (wt ? HTTP_BLOCK : HTTP_NONE),
                 "wb", HTTP_NONE);
    // Property name
    page->printf(HTTP_TEXT_FIELD, "Property name:", "pn", "8",
                 (this->editingName != nullptr
                      ? this->editingName->c_str()
                      : getNextName(GPIO_TYPE_GROUPED, "on").c_str()));
    // showAsWebthingDevice
    page->printf(HTTP_CHECKBOX_OPTION, "wt", "wt", (wt ? HTTP_CHECKED : ""),
                 "tgw()", "Webthing property");
    page->printf(HTTP_DIV_ID_BEGIN, "wa");
    // Webthing title
    page->printf(HTTP_TEXT_FIELD, "Webthing title:", "wtt", "12",
                 (this->editingTitle != nullptr
                      ? this->editingTitle->c_str()
                      : getNextName(GPIO_TYPE_GROUPED, "Switch").c_str()));
    page->print(FPSTR(HTTP_DIV_END));
    page->printf(HTTP_TOGGLE_FUNCTION_SCRIPT, "tgw()", "wt", "wa", "wb");
    // Submit, Cancel
    page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
    page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
  }

  void handleHttpAddGpioSwitchButton(AsyncWebServerRequest* request,
                                     Print* page) {
    updateEditingItem(request, page);
    // GPIO
    addGpioChooser(page, false);
    // Output configuration
    byte linkedGpio =
        (this->editingItem != nullptr
             ? this->editingItem->getByteArrayValue(BYTE_LINKED_GPIO)
             : getFirstUnusedGpioOutput());
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "ma",
                 (linkedGpio != BYTE_NOT_LINKED_GPIO ? HTTP_BLOCK : HTTP_NONE),
                 "mb",
                 (linkedGpio == BYTE_NOT_LINKED_GPIO ? HTTP_BLOCK : HTTP_NONE));
    page->printf(HTTP_CHECKBOX_OPTION, "mq", "mq",
                 (linkedGpio != BYTE_NOT_LINKED_GPIO ? HTTP_CHECKED : ""),
                 "tg()", "Switch directly");
    // Option A: Switch other GPIO
    page->printf(HTTP_DIV_ID_BEGIN, "ma");
    addLinkedGpioChooser(page);
    page->print(FPSTR(HTTP_DIV_END));
    // Option B: Toggle property
    page->printf(HTTP_DIV_ID_BEGIN, "mb");
    page->printf(
        HTTP_TEXT_FIELD, "Property name:", "pn", "8",
        ((this->editingName != nullptr) && (linkedGpio == BYTE_NOT_LINKED_GPIO)
             ? this->editingName->c_str()
             : getNextName(GPIO_TYPE_SWITCH, "switch").c_str()));
    page->print(FPSTR(HTTP_DIV_END));
    page->printf(HTTP_TOGGLE_FUNCTION_SCRIPT, "tg()", "mq", "ma", "mb");
  }

  void handleHttpSubmitUseTemplate(AsyncWebServerRequest* request,
                                   Print* page) {
    this->configureTemplate(request->arg("dt").toInt());
  }

  void handleHttpSubmitAddGpioLed(AsyncWebServerRequest* request, Print* page) {
    String voo = request->arg("voo");
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));    
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));

    addLed(request->arg("gp").toInt(), gr, mq, wt,
           (request->arg("iv") == HTTP_TRUE),
           (!gr ? request->arg("pn") : voo), 
           (!gr ? request->arg("wtt") : request->arg("mot")),
           (request->arg("ls") == HTTP_TRUE));
  }

  void handleHttpSubmitAddGpioRelay(AsyncWebServerRequest* request,
                                    Print* page) {
    String voo = request->arg("voo");
    bool gr = ((!voo.equals("0")) && (!voo.equals("1")) && (!voo.equals("2")));    
    bool mq = ((!gr) && ((voo.equals("1")) || (voo.equals("2"))));
    bool wt = ((!gr) && (voo.equals("2")));


    addRelay(request->arg("gp").toInt(), gr, mq, wt,
             (request->arg("iv") == HTTP_TRUE), 
             (!gr ? request->arg("pn") : voo),
             (!gr ? request->arg("wtt") : request->arg("mot")));
  }

  void handleHttpSubmitAddOnOffMode(AsyncWebServerRequest* request,
                                    Print* page) {
    addOnOffMode((request->arg("wt") == HTTP_TRUE), 
                 request->arg("pn"),
                 request->arg("wtt"));
  }

  void handleHttpSubmitAddGpioSwitch(AsyncWebServerRequest* request,
                                     Print* page) {
    bool switchDirect = (request->arg("mq") == HTTP_TRUE);
    addSwitch(
        request->arg("gp").toInt(),
        (switchDirect ? request->arg("lgp").toInt() : BYTE_NOT_LINKED_GPIO),
        (switchDirect ? "" : request->arg("pn")));
  }

  void handleHttpSubmitAddGpioButton(AsyncWebServerRequest* request,
                                     Print* page) {
    bool switchDirect = (request->arg("mq") == HTTP_TRUE);
    addButton(
        request->arg("gp").toInt(),
        (switchDirect ? request->arg("lgp").toInt() : BYTE_NOT_LINKED_GPIO),
        (switchDirect ? "" : request->arg("pn")),
        (request->arg("iv") == HTTP_TRUE), (request->arg("lp") == HTTP_TRUE));
  }

  void handleHttpRemoveGpioButton(AsyncWebServerRequest* request, Print* page) {
    bool exists = (request->hasParam("gpio", true));
    if (exists) {
      String sIndex = request->getParam("gpio", true)->value();
      int index = atoi(sIndex.c_str());
      this->removeGpioConfig(index);
      // moveProperty(index + 1, MAX_GPIOS);
      page->print("Deleted item: ");
      page->print(sIndex);
    } else {
      page->print("Missing GPIO value. Nothing deleted.");
    }
    page->printf(HTTP_BUTTON, DEVICE_ID, "get", "Go back");
  }

  virtual void printConfigPage(AsyncWebServerRequest* request, Print* page) {
    if (numberOfGPIOs->getByte() < MAX_GPIOS) {
      page->print(F("<table  class='tt'>"));
      tr(page);
      page->print(F("<td colspan='3'>"));
      page->printf(HTTP_BUTTON, HTTP_USE_TEMPLATE, "get", "Use a template...");
      tdEnd(page);
      trEnd(page);
      tr(page);
      td(page);
      page->print(F("Add output:"));
      tdEnd(page);
      // LED
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_LED, "get", "LED");
      tdEnd(page);
      // Relay
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_RELAY, "get", "Relay");
      tdEnd(page);
      // Switch
      trEnd(page);
      tr(page);
      td(page);
      page->print(F("Add input:"));
      tdEnd(page);
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_SWITCH, "get", "Switch");
      tdEnd(page);
      // Button
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_BUTTON, "get", "Button");
      tdEnd(page);
      trEnd(page);
      tr(page);
      td(page);
      page->print(F("Add grouped:"));
      tdEnd(page);
      td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_GROUPED, "get", "OnOff");
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
    for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);
      sprintf(pName, "gpio_%d", i);
      sprintf(pNumber, "%d", i);
      tr(page);
      td(page);
      page->print(pNumber);
      tdEnd(page);
      td(page);
      char* targetName = HTTP_ADD_LED;
      switch (gConfig->getByteArrayValue(BYTE_TYPE)) {
        case GPIO_TYPE_LED:
          page->print("LED");
          break;
        case GPIO_TYPE_RELAY:
          page->print("Relay");
          targetName = HTTP_ADD_RELAY;
          break;
        case GPIO_TYPE_BUTTON:
          page->print("Button");
          targetName = HTTP_ADD_BUTTON;
          break;
        case GPIO_TYPE_SWITCH:
          page->print("Switch");
          targetName = HTTP_ADD_SWITCH;
          break;
        case GPIO_TYPE_GROUPED:
          page->print("OnOff");
          targetName = HTTP_ADD_GROUPED;
          break;
      }
      tdEnd(page);
      td(page);
      if (gConfig->getByteArrayValue(BYTE_TYPE) != GPIO_TYPE_GROUPED) {
        sprintf(gNumber, "%d", gConfig->getByteArrayValue(BYTE_GPIO));
        page->print(gNumber);
      }
      tdEnd(page);
      td(page);
      if (this->isGpioAnOutput(gConfig->getByteArrayValue(BYTE_TYPE))) {
        WProperty* gName = getGpioName(i);
        page->print(gName->c_str());
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

    page->printf(HTTP_CONFIG_PAGE_BEGIN, getId());
    page->print(FPSTR(HTTP_CONFIG_SAVE_BUTTON));
  }

  void saveConfigPage(AsyncWebServerRequest* request, Print* page) {}

 protected:
  WProperty* getGpioConfig(byte index) {
    String pNumber = "^g";
    pNumber.concat(index);    
    return network->getSettings()->getSetting(pNumber.c_str());
  }

  WProperty* addGpioConfig() {
    byte index = this->numberOfGPIOs->getByte();
    String pNumber = "^g";
    pNumber.concat(index);
    // Config    
    WProperty* gConfig = network->getSettings()->setByteArray(pNumber.c_str(), DEFAULT_PROP_ARRAY);
    byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
    pNumber.concat(".");
    for (byte i = 0; i < getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);
      network->getSettings()->setString(subNumber.c_str(), "");
    }
    return gConfig;
  }

  void removeGpioConfig(byte index) {
    WProperty* gConfig = getGpioConfig(index);
    byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
    String pNumber = "^g";
    pNumber.concat(index);
    // Config    
    network->getSettings()->remove(pNumber.c_str());
    pNumber.concat(".");
    for (byte i = 0; i < getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);      
      network->getSettings()->remove(subNumber.c_str());
    }  
    //Move next gpios 1 place forward
    
    for (int i = index + 1; i < this->numberOfGPIOs->getByte(); i++) {
      gConfig = getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      // Config
      pNumber = "^g";
      pNumber.concat(i);
      String p2 = = "^g";
      p2.concat(i - 1);      
      network->getSettings()->getSetting(pNumber.c_str())->setId(p2.c_str());
      pNumber.concat(".");
      p2.concat(".");
      for (byte b = 0; b < getNumberOfChars(gType); b++) {
        String subNumber = pNumber;
        subNumber.concat(b);        
        String subP2 = p2;
        subP2.concat(b);
        network->getSettings()->getSetting(subNumber.c_str())->setId(subP2.c_str());
      }
    }
    this->numberOfGPIOs->setByte(this->numberOfGPIOs->getByte() - 1);
  }

  void updateEditingItem(AsyncWebServerRequest* request, Print* page) {
    bool exists = (request->hasParam("gpio", true));
    if (!exists) {
      page->print("New item");
      clearEditingItem();
    } else {
      String sIndex = request->getParam("gpio", true)->value();
      int index = atoi(sIndex.c_str());
      page->print("Edit item: ");
      page->print(sIndex);
      this->editingIndex = index;
      this->editingItem = getGpioConfig(index);
    }
  }

  void ensureEditingItem() {
    if (this->editingIndex == NO_INDEX) {            
      this->editingIndex = this->numberOfGPIOs->getByte();
      this->editingItem = this->addGpioConfig();
      this->numberOfGPIOs->setByte(this->editingIndex + 1);
    }
  }

  bool isGpioFree(byte gpio) {
    // exclude already used GPIOs
    if ((this->editingItem != nullptr) &&
        (this->editingItem->getByteArrayValue(BYTE_GPIO) == gpio)) {
      return true;
    } else {
      for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
        if (getGpioConfig(i)->getByteArrayValue(BYTE_GPIO) == gpio) {
          return false;
        }
      }
    }
    return true;
  }

  bool isGpioAnOutput(byte gType) {
    return ((gType == GPIO_TYPE_LED) || (gType == GPIO_TYPE_RELAY));
  }

  bool isGpioAnInput(byte gType) {
    return ((gType == GPIO_TYPE_BUTTON) || (gType == GPIO_TYPE_SWITCH));
  }

  bool isGpioGrouped(byte gType) { return (gType == GPIO_TYPE_GROUPED); }

  bool addGpioChooserItem(Print* page, byte gpio, const char* title) {
    int aGpio = (this->editingItem != nullptr
                     ? this->editingItem->getByteArrayValue(BYTE_GPIO)
                     : 13);
    char* pNumber = new char[2];
    sprintf(pNumber, "%d", gpio);
    if (isGpioFree(gpio))
      page->printf(HTTP_COMBOBOX_ITEM, pNumber,
                   (aGpio == gpio ? HTTP_SELECTED : ""), title);
    return true;
  }

  void addGpioChooser(Print* page, bool isOutput) {
    page->printf(HTTP_COMBOBOX_BEGIN, "GPIO:", "gp");
#ifdef ESP8266
    addGpioChooserItem(page, 0, "0");
    if (isOutput) {
      addGpioChooserItem(page, 1, "1 (TX)");
    }
    addGpioChooserItem(page, 2, "2");
    if (!isOutput) {
      addGpioChooserItem(page, 3, "3 (RX)");
    }
    addGpioChooserItem(page, 4, "4");
    addGpioChooserItem(page, 5, "5");
    addGpioChooserItem(page, 12, "12");
    addGpioChooserItem(page, 13, "13");
    addGpioChooserItem(page, 14, "14");
    if (isOutput) addGpioChooserItem(page, 15, "15");
#elif ESP32

#endif
    page->print(FPSTR(HTTP_COMBOBOX_END));
  }

  byte getFirstUnusedGpioOutput() {
    for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      if (isGpioAnOutput(gType)) {
        byte gGpio = gConfig->getByteArrayValue(BYTE_GPIO);
        bool freeGpio = true;
        for (byte b = 0; (free) && (b < this->numberOfGPIOs->getByte()); b++) {
          WProperty* gConfig2 = getGpioConfig(b);
          byte gType2 = gConfig2->getByteArrayValue(BYTE_TYPE);
          if (isGpioAnInput(gType2)) {
            if ((gConfig2->getByteArrayValue(BYTE_LINKED_GPIO) == gGpio)) {
              freeGpio = false;
            }
          }
        }
        if (free) {
          return gGpio;
          break;
        }
      }
    }
    return BYTE_NOT_LINKED_GPIO;
  }

  String getNextName(byte aType, String baseName) {
    int noMax = 0;
    for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      if (aType == gType) {
        String gName = String(getGpioName(i)->c_str());
        int b = gName.lastIndexOf("_");
        if (b > -1) {
          noMax =
              max(noMax, (int)gName.substring(b + 1, gName.length()).toInt());
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

  void addLinkedGpioChooser(Print* page) {
    // BYTE_SWITCH_LINKED_GPIO
    page->printf(HTTP_COMBOBOX_BEGIN, "Select output GPIO:", "lgp");
    byte linkedGpio =
        (this->editingItem != nullptr
             ? this->editingItem->getByteArrayValue(BYTE_LINKED_GPIO)
             : BYTE_NOT_LINKED_GPIO);
    // page->printf(HTTP_COMBOBOX_ITEM, "255", (BYTE_NOT_LINKED_GPIO ==
    // linkedGpio ? HTTP_SELECTED : ""), "None");
    for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      byte gGpio = gConfig->getByteArrayValue(BYTE_GPIO);
      if (isGpioAnOutput(gType)) {
        char* pNumber = new char[2];
        sprintf(pNumber, "%d", gGpio);
        String iName = String(pNumber);
        iName.concat(" (");
        iName.concat(getGpioName(i)->c_str());
        iName.concat(")");
        page->printf(HTTP_COMBOBOX_ITEM, pNumber,
                     (gGpio == linkedGpio ? HTTP_SELECTED : ""), iName.c_str());
      }
    }
    page->print(FPSTR(HTTP_COMBOBOX_END));
  }

  void printVisibility(Print* page, byte aProps, byte aType,
                          String baseName) {
    bool gr = bitRead(aProps, BIT_CONFIG_PROPERTY_GROUPED);
    bool mq = bitRead(aProps, BIT_CONFIG_PROPERTY_MQTT);
    bool wt = bitRead(aProps, BIT_CONFIG_PROPERTY_WEBTHING);

    // ComboBox
    page->printf(HTTP_COMBOBOX_OPTION_BEGIN, "Property:", "voo");
    // MQTT and Webthings
    WHtml::comboBoxItem(page, "MQTT and Webthings", "2", !gr && mq && wt);    
    // MQTT
    WHtml::comboBoxItem(page, "Only MQTT", "1", !gr && mq && !wt);    
    // None
    WHtml::comboBoxItem(page, "No property", "0", !gr && !mq && !wt);    

    // List grouped properties
    for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);

      if (gConfig->getByteArrayValue(BYTE_TYPE) == GPIO_TYPE_GROUPED) {
        WProperty* gName = this->getGpioName(i);
        WProperty* gTitle = this->getGpioTitle(i);
        WHtml::comboBoxItem(page, gName->c_str(), gName->c_str(), gr && this->editingName != nullptr && this->editingName->equalsString(gName->c_str()));      
      }
    }
    page->print(FPSTR(HTTP_COMBOBOX_END));
    page->print(FPSTR(HTTP_COMBO_BOX_FUNCTION_SCRIPT));
    // MQTT property name
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "ma",
                 (!gr && mq ? HTTP_BLOCK : HTTP_NONE), "mb", HTTP_NONE);        
    page->printf(HTTP_DIV_ID_BEGIN, "ma");
    WHtml::textField(page, "pn", "Property name:", 8, 
                     (this->editingName != nullptr ? this->editingName->c_str()
                                      : getNextName(aType, baseName).c_str()));
    page->print(FPSTR(HTTP_DIV_END));
    // Webthing title
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "wa",
                 (!gr && wt ? HTTP_BLOCK : HTTP_NONE), "wb", HTTP_NONE);
    page->printf(HTTP_DIV_ID_BEGIN, "wa");
    WHtml::textField(page, "wtt", "Webthing title:", 12, 
                     (this->editingTitle != nullptr ? this->editingTitle->c_str()
                                       : getNextName(aType, baseName).c_str()));    
    page->print(FPSTR(HTTP_DIV_END));
    // Mode name
    page->printf(HTTP_TOGGLE_GROUP_STYLE, "na",
                 (gr ? HTTP_BLOCK : HTTP_NONE), "nb", HTTP_NONE);
    page->printf(HTTP_DIV_ID_BEGIN, "na");
    WHtml::textField(page, "mot", "Mode name:", 12, 
                     (this->editingTitle != nullptr ? this->editingTitle->c_str()
                                       : getNextName(aType, baseName).c_str()));    
    page->print(FPSTR(HTTP_DIV_END));
  }

 private:
  WProperty* numberOfGPIOs;
  WProperty* editingItem;
  byte editingIndex;  

  void configureDevice() {
    // 1. Grouped
    for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);
      WProperty* gName = this->getSubString(gConfig, 0);
      WProperty* gTitle = this->getSubString(gConfig, 1);
      if (gConfig->getByteArrayValue(BYTE_TYPE) == GPIO_TYPE_GROUPED) {
        bool mq = true;
        bool wt = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
        WProperty* onOffProp = WProperty::createOnOffProperty(gName->c_str(), gTitle->c_str());
        onOffProp->setVisibilityMqtt(mq);
        onOffProp->setVisibilityWebthing(wt);
        this->addProperty(onOffProp);
        WProperty* modeProp = WProperty::createStringProperty("moda", "Moda");
        modeProp->setVisibilityMqtt(mq);
        modeProp->setVisibilityWebthing(wt);
        this->addProperty(modeProp);
      }
    }
    // 1. only outputs
    for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);
      WProperty* gName = this->getSubString(gConfig, 0);
      WProperty* gTitle = this->getSubString(gConfig, 1);
      switch (gConfig->getByteArrayValue(BYTE_TYPE)) {
        case GPIO_TYPE_LED: {
          WLed* led = new WLed(gConfig->getByteArrayValue(BYTE_GPIO));
          this->addPin(led);
          bool gr = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_GROUPED);
          bool mq = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT);
          bool wt = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
          if (gr) {
            WProperty* mProp = this->getPropertyById("moda");
            mProp->addEnumString(gTitle->c_str());
          } else if ((mq) || (wt)) {
            WProperty* ledProp = WProperty::createOnOffProperty(
                gName->c_str(), gTitle->c_str());
            ledProp->setVisibilityMqtt(mq);
            ledProp->setVisibilityWebthing(wt);
            this->addProperty(ledProp);
            led->setProperty(ledProp);
          }
          // Inverted
          led->setInverted(gConfig->getByteArrayBitValue(
              BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
          // Linkstate
          if (gConfig->getByteArrayBitValue(BYTE_CONFIG,
                                            BIT_CONFIG_LED_LINKSTATE)) {
            network->setStatusLed(led, false);
          }
          break;
        }
        case GPIO_TYPE_RELAY: {
          WRelay* relay = new WRelay(gConfig->getByteArrayValue(BYTE_GPIO));
          this->addPin(relay);
          bool gr = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_GROUPED);
          bool mq = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT);
          bool wt = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING);
          if (gr) {
            WProperty* mProp = this->getPropertyById("moda");
            if (mProp != nullptr) {
              mProp->addEnumString(gTitle->c_str());
            }  
          } else if ((mq) || (wt)) {
            WProperty* relayProp = WProperty::createOnOffProperty(
                gName->c_str(), gTitle->c_str());
            relayProp->setVisibilityMqtt(mq);
            relayProp->setVisibilityWebthing(wt);
            this->addProperty(relayProp);
            relay->setProperty(relayProp);
          }
          // Inverted
          relay->setInverted(gConfig->getByteArrayBitValue(
              BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
          break;
        }
      }
    }
    // 2. inputs
    for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);
      WProperty* gName = this->getSubString(gConfig, 0);
      byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
      if ((gType == GPIO_TYPE_BUTTON) || (gType == GPIO_TYPE_SWITCH)) {
        byte bMode = (gType == GPIO_TYPE_SWITCH
                          ? MODE_SWITCH
                          : (gConfig->getByteArrayBitValue(
                                 BYTE_CONFIG, BIT_CONFIG_SWITCH_AP_LONGPRESS)
                                 ? MODE_BUTTON_LONG_PRESS
                                 : MODE_BUTTON));
        WSwitch* button =
            new WSwitch(gConfig->getByteArrayValue(BYTE_GPIO), bMode);
        // Inverted
        button->setInverted(gConfig->getByteArrayBitValue(
            BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
        if (bMode == BIT_CONFIG_SWITCH_AP_LONGPRESS) {
          button->setOnLongPressed([this]() {
            if (!network->isSoftAP()) {
              network->getSettings()->forceAPNextStart();
              delay(200);
              ESP.restart();
            }
          });
        }
        this->addPin(button);
        byte linkedGpio = gConfig->getByteArrayValue(BYTE_LINKED_GPIO);
        if (linkedGpio != BYTE_NOT_LINKED_GPIO) {
          // search property assigned to the GPIO
          for (byte b = 0;
               (!button->hasProperty()) && (b < this->numberOfGPIOs->getByte());
               b++) {
            WProperty* gConfig2 = getGpioConfig(b);
            byte gType2 = gConfig2->getByteArrayValue(BYTE_TYPE);
            byte gGpio2 = gConfig2->getByteArrayValue(BYTE_GPIO);
            WProperty* gName2 = this->getSubString(gConfig2, 0);
            if ((!gName2->equalsString("")) && (isGpioAnOutput(gType2)) &&
                (gGpio2 == linkedGpio)) {
              button->setProperty(this->getPropertyById(gName2->c_str()));
            }
          }
        } else if (!gName->equalsString("")) {
          // Only trigger via MQTT
          WProperty* triggerProperty =
              WProperty::createOnOffProperty(gName->c_str(), gName->c_str());
          triggerProperty->setVisibility(MQTT);
          this->addProperty(triggerProperty);
          button->setTriggerProperty(triggerProperty);
        }
      }
    }
  }

  void addGpioConfigItem(byte type, byte gpio, bool grouped, bool mqtt, bool webthing, bool inverted) {
    ensureEditingItem();
    this->editingItem->setByteArrayValue(BYTE_TYPE, type);
    this->editingItem->setByteArrayValue(BYTE_GPIO, gpio);
    this->editingItem->setByteArrayBitValue(BYTE_CONFIG,
                                            BIT_CONFIG_PROPERTY_GROUPED, grouped);
    this->editingItem->setByteArrayBitValue(BYTE_CONFIG,
                                            BIT_CONFIG_PROPERTY_MQTT, mqtt);
    this->editingItem->setByteArrayBitValue(
        BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING, webthing);
    this->editingItem->setByteArrayBitValue(BYTE_CONFIG,
                                            BIT_CONFIG_LED_INVERTED, inverted);
    String pNumber = this->editingItem->getId();                                            
    pNumber.concat(".");
    //char* pNumber = new char[6];    
    byte gType = this->editingItem->getByteArrayValue(BYTE_TYPE);
    for (byte i = 0; i < getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);
      network->getSettings()->setString(subNumber.c_str(), "");
    }

    //this->editingName->setString(oName.c_str());
    //this->editingTitle->setString(oTitle.c_str());
  }

  void clearEditingItem() {
    this->editingItem = nullptr;
    this->editingIndex = NO_INDEX;
  }

  WProperty* setSubString(WProperty* gpioConfig, byte subIndex, String value) {
    String pNumber = gpioConfig->getId(); 
    pNumber.concat(".");
    pNumber.concat(subIndex);
    return network->getSettings()->setString(pNumber.c_str(), value.c_str());
  }

  WProperty* getSubString(WProperty* gpioConfig, byte subIndex) {
    String pNumber = gpioConfig->getId(); 
    pNumber.concat(".");
    pNumber.concat(subIndex);
    return network->getSettings()->getSetting(pNumber.c_str());
	}

  void addRelay(byte gpio, bool gr, bool mqtt, bool webthing, bool inverted,
                String oName, String oTitle) {
    addGpioConfigItem(GPIO_TYPE_RELAY, gpio, gr, mqtt, webthing, inverted);
    setSubString(this->editingItem, 0, oName);
    setSubString(this->editingItem, 1, oTitle);
    clearEditingItem();
  }

  void addLed(byte gpio, bool gr, bool mqtt, bool webthing, bool inverted, String oName,
              String oTitle, bool linkState) {
    addGpioConfigItem(GPIO_TYPE_LED, gpio, gr, mqtt, webthing, inverted);
    setSubString(this->editingItem, 0, oName);
    setSubString(this->editingItem, 1, oTitle);
    this->editingItem->setByteArrayBitValue(
        BYTE_CONFIG, BIT_CONFIG_LED_LINKSTATE, linkState);
    clearEditingItem();
  }

  void addOnOffMode(bool webthing, String oName, String oTitle) {
    addGpioConfigItem(GPIO_TYPE_GROUPED, BYTE_NOT_LINKED_GPIO, false, true, webthing,
                      false);
    setSubString(this->editingItem, 0, oName);
    setSubString(this->editingItem, 1, oTitle);
    clearEditingItem();
  }

  void addSwitch(byte gpio, byte linkedGpio) {
    addGpioConfigItem(GPIO_TYPE_SWITCH, gpio, false, false, false, false);
    this->editingItem->setByteArrayValue(BYTE_LINKED_GPIO, linkedGpio);
    clearEditingItem();
  }

  void addButton(byte gpio, byte linkedGpio, bool inverted,
                 bool switchApLongPress) {
    addGpioConfigItem(GPIO_TYPE_BUTTON, gpio, false, false, false, false);
    this->editingItem->setByteArrayValue(BYTE_LINKED_GPIO, linkedGpio);
    this->editingItem->setByteArrayBitValue(BYTE_CONFIG,
                                            BIT_CONFIG_LED_INVERTED, inverted);
    this->editingItem->setByteArrayBitValue(
        BYTE_CONFIG, BIT_CONFIG_SWITCH_AP_LONGPRESS, switchApLongPress);
    clearEditingItem();
  }

  byte getNumberOfChars(byte gType) {
    switch (gType) {
      case GPIO_TYPE_GROUPED : return 4;
      case GPIO_TYPE_LED : return 2;
      case GPIO_TYPE_RELAY : return 2;
      default: return 0;
    }
  }

  void configureTemplate(byte templateIndex) {
    // clear all GPIOs
    for (int i = this->numberOfGPIOs->getByte() - 1; i >= 0; i--) {
      this->removeGpioConfig(i);
    }
    switch (templateIndex) {
      case 0: {
        // Neo Coolcam
        this->addLed(4, false, false, false, false, "led", "", true);
        this->addRelay(12, false, true, true, false, "on", "on");
        this->addButton(13, 12, false, true);
        break;
      }
      case 1: {
        // Milos 1-fach
        this->addLed(4, false, false, false, true, "led", "", true);
        this->addRelay(13, false, true, true, false, "on", "on");
        this->addSwitch(12, 13);
        break;
      }
      case 2: {
        // Milos 2-fach
        this->addLed(4, false, false, false, true, "led", "", true);
        this->addRelay(13, false, true, true, false, "on", "on");
        this->addRelay(15, false, true, true, false, "on_2", "on_2");
        this->addSwitch(12, 13);
        this->addSwitch(5, 15);
        break;
      }
      case 3: {
        // Sonoff mini + Switch
        this->addLed(13, false, false, false, false, "led", "", true);
        this->addRelay(12, false, true, true, false, "on", "on");
        this->addButton(0, 12, true, true);
        this->addSwitch(4, 12);
        break;
      }
      case 4: {
        // Sonoff mini + Button
        this->addLed(13, false, false, false, false, "led", "", true);
        this->addRelay(12, false, true, true, false, "on", "on");
        this->addButton(0, 12, true, true);
        this->addButton(4, 12, true, false);
        break;
      }
      case 5: {
        // Sonoff Basic
        this->addLed(13, false, false, false, false, "led", "", true);
        this->addRelay(12, false, true, true, false, "on", "on");
        this->addButton(0, 12, false, true);
        break;
      }
      case 6: {
        // Sonoff 4-channel
        this->addLed(13, false, false, false, false, "led", "", true);
        this->addRelay(12, false, true, true, false, "on", "on");
        this->addRelay(5, false, true, true, false, "on_2", "on_2");
        this->addRelay(4, false, true, true, false, "on_3", "on_3");
        this->addRelay(15, false, true, true, false, "on_4", "on_4");
        this->addButton(0, 12, false, true);
        this->addButton(9, 5, false, false);
        this->addButton(10, 4, false, false);
        this->addButton(14, 15, false, false);
        break;
      }
      case 7: {
        // Wemos: Relay at D1, Switch at D3
        this->addLed(2, false, false, false, false, "led", "", true);
        this->addRelay(5, false, true, true, false, "on", "on");
        this->addButton(0, 5, false, true);
        break;
      }
    }
  }
};

#endif
