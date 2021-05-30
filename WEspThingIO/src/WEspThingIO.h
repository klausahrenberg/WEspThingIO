#ifndef W_ESP_THING_IO_H
#define W_ESP_THING_IO_H

#include "WDevice.h"
#include "WSwitch.h"
#include "WRelay.h"
#include "WProperty.h"
#include "WPage.h"

#define DEVICE_ID "switch"
#define MAX_GPIOS 12
#define MAX_PROP_BYTES 4
#define GPIO_TYPE_LED 0
#define GPIO_TYPE_RELAY 1
#define GPIO_TYPE_BUTTON 2
#define GPIO_TYPE_SWITCH 3

#define BYTE_TYPE 0
#define BYTE_GPIO 1
#define BYTE_CONFIG 2
#define BYTE_LINKED_GPIO 3
#define BYTE_NOT_LINKED_GPIO 0xFF

#define BIT_CONFIG_MQTT 0
#define BIT_CONFIG_WEBTHING 1
#define BIT_CONFIG_LED_INVERTED 2
#define BIT_CONFIG_LED_LINKSTATE 3
#define BIT_CONFIG_SWITCH_AP_LONGPRESS 3

#define CAPTION_EDIT "Edit"
#define CAPTION_REMOVE "Remove"
#define CAPTION_OK "Ok"
#define CAPTION_CANCEL "Cancel"
#define HTTP_USE_TEMPLATE "usetemplate"
#define HTTP_ADD_LED "addled"
#define HTTP_ADD_RELAY "addrelay"
#define HTTP_ADD_BUTTON "addbutton"
#define HTTP_ADD_SWITCH "addswitch"
#define HTTP_REMOVE_GPIO "removegpio"

const byte *DEFAULT_PROP_ARRAY = (const byte[]){0, 0, 0, 0};
const char *DEFAULT_GPIO_ID = "_^g_%d";
const char *DEFAULT_GPIO_NAME = "_^n_%d";
const char *DEFAULT_GPIO_TITLE = "_^t_%d";
const static char HTTP_BUTTON_VALUE[]    PROGMEM = R"=====(
	<div>
  	<form action='/%s' method='POST'>
    	<button class='%s' name='gpio' value='%s'>%s</button>
    </form>
  </div>
)=====";

class WEspThingIO: public WDevice {
public:
	WEspThingIO(WNetwork* network)
	    	: WDevice(network, DEVICE_ID, DEVICE_ID, DEVICE_TYPE_ON_OFF_SWITCH) {
		this->editingItem = nullptr;
		this->editingName = nullptr;
		this->editingTitle = nullptr;
	  this->numberOfGPIOs = network->getSettings()->setByte("numberOfGPIOs", 0, MAX_GPIOS);
		this->numberOfGPIOs->setVisibility(NONE);
		this->addProperty(this->numberOfGPIOs);
		byte nog = this->numberOfGPIOs->getByte();
		this->numberOfGPIOs->setByte(0);
		for (byte i = 0; i < nog; i++) {
			this->addGpioConfig();
			this->numberOfGPIOs->setByte(i + 1);
		}
		//Configure Device
		configureDevice();
		this->setVisibility(ALL);//this->showAsWebthingDevice->getBoolean() ? ALL : MQTT);
		//HtmlPages
    WPage* configPage = new WPage(this->getId(), "Configure device");
    configPage->setPrintPage(std::bind(&WEspThingIO::printConfigPage, this, std::placeholders::_1, std::placeholders::_2));
    configPage->setSubmittedPage(std::bind(&WEspThingIO::saveConfigPage, this, std::placeholders::_1, std::placeholders::_2));
    network->addCustomPage(configPage);
		//Add LED
		WPage* utPage = new WPage(HTTP_USE_TEMPLATE, "Use a template");
		utPage->setShowInMainMenu(false);
    utPage->setPrintPage(std::bind(&WEspThingIO::handleHttpUseTemplate, this, std::placeholders::_1, std::placeholders::_2));
    utPage->setSubmittedPage(std::bind(&WEspThingIO::handleHttpSubmitUseTemplate, this, std::placeholders::_1, std::placeholders::_2));
		utPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(utPage);
		//Add LED
		WPage* ledPage = new WPage(HTTP_ADD_LED, "Add/edit LED");
		ledPage->setShowInMainMenu(false);
    ledPage->setPrintPage(std::bind(&WEspThingIO::handleHttpAddGpioLed, this, std::placeholders::_1, std::placeholders::_2));
    ledPage->setSubmittedPage(std::bind(&WEspThingIO::handleHttpSubmitAddGpioLed, this, std::placeholders::_1, std::placeholders::_2));
		ledPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(ledPage);
		//Add Relay
		WPage* relayPage = new WPage(HTTP_ADD_RELAY, "Add/edit Relay");
		relayPage->setShowInMainMenu(false);
    relayPage->setPrintPage(std::bind(&WEspThingIO::handleHttpAddGpioRelay, this, std::placeholders::_1, std::placeholders::_2));
    relayPage->setSubmittedPage(std::bind(&WEspThingIO::handleHttpSubmitAddGpioRelay, this, std::placeholders::_1, std::placeholders::_2));
		relayPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(relayPage);
		//Add Switch
		WPage* switchPage = new WPage(HTTP_ADD_SWITCH, "Add/edit Switch");
		switchPage->setShowInMainMenu(false);
    switchPage->setPrintPage(std::bind(&WEspThingIO::handleHttpAddGpioSwitch, this, std::placeholders::_1, std::placeholders::_2));
    switchPage->setSubmittedPage(std::bind(&WEspThingIO::handleHttpSubmitAddGpioSwitch, this, std::placeholders::_1, std::placeholders::_2));
		switchPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(switchPage);
		//Add Switch
		WPage* buttonPage = new WPage(HTTP_ADD_BUTTON, "Add/edit Button");
		buttonPage->setShowInMainMenu(false);
    buttonPage->setPrintPage(std::bind(&WEspThingIO::handleHttpAddGpioButton, this, std::placeholders::_1, std::placeholders::_2));
    buttonPage->setSubmittedPage(std::bind(&WEspThingIO::handleHttpSubmitAddGpioButton, this, std::placeholders::_1, std::placeholders::_2));
		buttonPage->setTargetAfterSubmitting(configPage);
    network->addCustomPage(buttonPage);
		//Remove GPIO
		WPage* removeGpio = new WPage(HTTP_REMOVE_GPIO, "Remove GPIO");
		removeGpio->setShowInMainMenu(false);
    removeGpio->setPrintPage(std::bind(&WEspThingIO::handleHttpRemoveGpioButton, this, std::placeholders::_1, std::placeholders::_2));
    //ledPage->setSubmittedPage(std::bind(&WEspThingIO::submittedAddLedPage, this, std::placeholders::_1, std::placeholders::_2));
    network->addCustomPage(removeGpio);
  }

	void handleHttpUseTemplate(AsyncWebServerRequest *request, Print* page) {
		page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_USE_TEMPLATE);
		//deviceType
		page->printf(HTTP_COMBOBOX_BEGIN, "Model:", "dt");
		page->printf(HTTP_COMBOBOX_ITEM, "0", HTTP_SELECTED, F("Neo Coolcam"));
		page->printf(HTTP_COMBOBOX_ITEM, "1", "", F("Milos Single Switch"));
		page->printf(HTTP_COMBOBOX_ITEM, "2", "", F("Milos Double Switch"));
		page->printf(HTTP_COMBOBOX_ITEM, "3", "", F("Sonoff Mini + Switch"));
		page->printf(HTTP_COMBOBOX_ITEM, "4", "", F("Sonoff Mini + Button"));
		page->printf(HTTP_COMBOBOX_ITEM, "5", "", F("Sonoff Basic"));
		page->printf(HTTP_COMBOBOX_ITEM, "6", "", F("Sonoff 4-channel"));
		page->printf(HTTP_COMBOBOX_ITEM, "7", "", F("Wemos: Relay at D1, Switch at D3"));
		page->print(FPSTR(HTTP_COMBOBOX_END));
		page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
		page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
	}

	void handleHttpAddGpioLed(AsyncWebServerRequest *request, Print* page) {
		page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_LED);
		updateEditingItem(request, page);
		//GPIO
		addGpioChooser(page, true);
		byte aProps = (this->editingItem != nullptr ? this->editingItem->getByteArrayValue(BYTE_CONFIG) : 0);
		printVisibility(page, aProps, GPIO_TYPE_LED, "led");
		//Inverted
		page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv", (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""), "", "Inverted");
		//Linkstate
		page->printf(HTTP_CHECKBOX_OPTION, "ls", "ls", (bitRead(aProps, BIT_CONFIG_LED_LINKSTATE) ? HTTP_CHECKED : ""), "", "Show network link state");
		page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
		page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
	}

	void handleHttpAddGpioRelay(AsyncWebServerRequest *request, Print* page) {
		page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_RELAY);
		updateEditingItem(request, page);
		//GPIO
		addGpioChooser(page, true);
		byte aProps = (this->editingItem != nullptr ? this->editingItem->getByteArrayValue(BYTE_CONFIG) : 0b00000011);
		printVisibility(page, aProps, GPIO_TYPE_RELAY, "on");
		//Inverted
		page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv", (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""), "", "Inverted");
		page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
		page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
	}

	void handleHttpAddGpioSwitch(AsyncWebServerRequest *request, Print* page) {
		page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_SWITCH);
		handleHttpAddGpioSwitchButton(request, page);
		page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
		page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
	}

	void handleHttpAddGpioButton(AsyncWebServerRequest *request, Print* page) {
		page->printf(HTTP_CONFIG_PAGE_BEGIN, HTTP_ADD_BUTTON);
		handleHttpAddGpioSwitchButton(request, page);
		byte aProps = (this->editingItem != nullptr ? this->editingItem->getByteArrayValue(BYTE_CONFIG) : 0);
		//Inverted
		page->printf(HTTP_CHECKBOX_OPTION, "iv", "iv", (bitRead(aProps, BIT_CONFIG_LED_INVERTED) ? HTTP_CHECKED : ""), "", "Inverted");
		page->printf(HTTP_CHECKBOX_OPTION, "lp", "lp", (bitRead(aProps, BIT_CONFIG_SWITCH_AP_LONGPRESS) ? HTTP_CHECKED : ""), "", "Open AccesPoint on long press");
		page->printf(HTTP_BUTTON_SUBMIT, CAPTION_OK);
		page->printf(HTTP_BUTTON, DEVICE_ID, "get", CAPTION_CANCEL);
	}

	void handleHttpAddGpioSwitchButton(AsyncWebServerRequest *request, Print* page) {
		updateEditingItem(request, page);
		//GPIO
		addGpioChooser(page, false);
		//Output configuration
		byte linkedGpio = (this->editingItem != nullptr ? this->editingItem->getByteArrayValue(BYTE_LINKED_GPIO) : getFirstUnusedGpioOutput());
		page->printf(HTTP_TOGGLE_GROUP_STYLE, "ma", (linkedGpio != BYTE_NOT_LINKED_GPIO ? HTTP_BLOCK : HTTP_NONE), "mb", (linkedGpio == BYTE_NOT_LINKED_GPIO ? HTTP_BLOCK : HTTP_NONE));
		page->printf(HTTP_CHECKBOX_OPTION, "mq", "mq", (linkedGpio != BYTE_NOT_LINKED_GPIO ? HTTP_CHECKED : ""), "tg()", "Switch directly");
		//Option A: Switch other GPIO
		page->printf(HTTP_DIV_ID_BEGIN, "ma");
		addLinkedGpioChooser(page);
		page->print(FPSTR(HTTP_DIV_END));
		//Option B: Toggle property
		page->printf(HTTP_DIV_ID_BEGIN, "mb");
		page->printf(HTTP_TEXT_FIELD, "Property name:", "pn", "8", ((this->editingName != nullptr) && (linkedGpio == BYTE_NOT_LINKED_GPIO) ? this->editingName->c_str() : getNextName(GPIO_TYPE_SWITCH, "switch").c_str()));
		page->print(FPSTR(HTTP_DIV_END));
		page->printf(HTTP_TOGGLE_FUNCTION_SCRIPT, "tg()", "mq", "ma", "mb");
	}

	void handleHttpSubmitUseTemplate(AsyncWebServerRequest *request, Print* page) {
		this->configureTemplate(request->arg("dt").toInt());
	}

	void handleHttpSubmitAddGpioLed(AsyncWebServerRequest *request, Print* page) {
		addLed(request->arg("gp").toInt(),
	         (request->arg("mq") == HTTP_TRUE), (request->arg("wt") == HTTP_TRUE),
					 (request->arg("iv") == HTTP_TRUE),
					 request->arg("pn"), request->arg("wtt"),
					 (request->arg("ls") == HTTP_TRUE));
	}

	void handleHttpSubmitAddGpioRelay(AsyncWebServerRequest *request, Print* page) {
		addRelay(request->arg("gp").toInt(),
	           (request->arg("mq") == HTTP_TRUE), (request->arg("wt") == HTTP_TRUE),
						 (request->arg("iv") == HTTP_TRUE),
						 request->arg("pn"), request->arg("wtt"));
	}

	void handleHttpSubmitAddGpioSwitch(AsyncWebServerRequest *request, Print* page) {
		bool switchDirect = (request->arg("mq") == HTTP_TRUE);
		addSwitch(request->arg("gp").toInt(),
	            (switchDirect ? request->arg("lgp").toInt() : BYTE_NOT_LINKED_GPIO),
						  (switchDirect ? "" : request->arg("pn")));
	}

	void handleHttpSubmitAddGpioButton(AsyncWebServerRequest *request, Print* page) {
		bool switchDirect = (request->arg("mq") == HTTP_TRUE);
		addButton(request->arg("gp").toInt(),
	            (switchDirect ? request->arg("lgp").toInt() : BYTE_NOT_LINKED_GPIO),
						  (switchDirect ? "" : request->arg("pn")),
						  (request->arg("iv") == HTTP_TRUE), (request->arg("lp") == HTTP_TRUE));
	}

	void handleHttpRemoveGpioButton(AsyncWebServerRequest *request, Print* page) {
		bool exists = (request->hasParam("gpio", true));
		if (exists) {
			String sIndex = request->getParam("gpio", true)->value();
			int index = atoi(sIndex.c_str());
			this->removeGpioConfig(index);
			//moveProperty(index + 1, MAX_GPIOS);
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
					page->printf(HTTP_BUTTON, HTTP_USE_TEMPLATE, "get", "Use a template..."); tdEnd(page);
				trEnd(page);
				tr(page);
				  td(page); page->print(F("Add output:")); tdEnd(page);
					//LED
					td(page); page->printf(HTTP_BUTTON, HTTP_ADD_LED, "get", "LED"); tdEnd(page);
					//Relay
					td(page); page->printf(HTTP_BUTTON, HTTP_ADD_RELAY, "get", "Relay"); tdEnd(page);
					//Switch
				trEnd(page);
				tr(page);
					td(page); page->print(F("Add input:")); tdEnd(page);
					td(page); page->printf(HTTP_BUTTON, HTTP_ADD_SWITCH, "get", "Switch"); tdEnd(page);
					//Button
					td(page); page->printf(HTTP_BUTTON, HTTP_ADD_BUTTON, "get", "Button"); tdEnd(page);
				trEnd(page);
				page->print(F("</table>"));
			}
			//Table with already stored MAX_GPIOS
			page->printf(HTTP_DIV_ID_BEGIN, "gd");
			page->print(F("<table  class='st'>"));
			tr(page);
				th(page); /*No*/ thEnd(page);
				th(page); page->print("Type"); thEnd(page);
				th(page); page->print("GPIO"); thEnd(page);
				th(page);	page->print("Name"); thEnd(page);
				th(page);	/*Edit*/ thEnd(page);
				th(page);	/*Remove*/ thEnd(page);
			trEnd(page);
			char* pName = new char[8];

			char* pNumber = new char[2];
			char* gNumber = new char[2];
			for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
				WProperty* gConfig = getGpioConfig(i);
				sprintf(pName, "gpio_%d", i);
				sprintf(pNumber, "%d", i);
				tr(page);
					td(page);	page->print(pNumber); tdEnd(page);
					td(page);
					char* targetName = HTTP_ADD_LED;
					switch (gConfig->getByteArrayValue(BYTE_TYPE)) {
						case GPIO_TYPE_LED : 	page->print("LED");	break;
						case GPIO_TYPE_RELAY : page->print("Relay"); targetName = HTTP_ADD_RELAY; break;
						case GPIO_TYPE_BUTTON : page->print("Button"); targetName = HTTP_ADD_BUTTON;	break;
						case GPIO_TYPE_SWITCH : page->print("Switch"); targetName = HTTP_ADD_SWITCH;
					}
					tdEnd(page);
					td(page);
					sprintf(gNumber, "%d", gConfig->getByteArrayValue(BYTE_GPIO));
					page->print(gNumber);
					tdEnd(page);
					td(page);
					//if ()
					WProperty* gName = getGpioName(i);
					page->print(gName->c_str());
					tdEnd(page);
					td(page);
					page->printf(HTTP_BUTTON_VALUE, targetName, "", pNumber, CAPTION_EDIT);
					tdEnd(page);
					td(page);
					page->printf(HTTP_BUTTON_VALUE, HTTP_REMOVE_GPIO, "cbtn", pNumber, CAPTION_REMOVE);
					tdEnd(page);
				trEnd(page);
			}
			page->print(F("</table>"));
			page->printf(HTTP_DIV_END);

			page->printf(HTTP_CONFIG_PAGE_BEGIN, getId());
			page->print(FPSTR(HTTP_CONFIG_SAVE_BUTTON));
	}

	void saveConfigPage(AsyncWebServerRequest* request, Print* page) {
	}

protected:

	WProperty* getGpioConfig(byte index) {
		char* pNumber = new char[6];
		sprintf(pNumber, DEFAULT_GPIO_ID, index);
		return network->getSettings()->getSetting(pNumber);
	}

	WProperty* getGpioName(byte index) {
		char* pNumber = new char[6];
		sprintf(pNumber, DEFAULT_GPIO_NAME, index);
		return network->getSettings()->getSetting(pNumber);
	}

	WProperty* getGpioTitle(byte index) {
		char* pNumber = new char[6];
		sprintf(pNumber, DEFAULT_GPIO_TITLE, index);
		return network->getSettings()->getSetting(pNumber);
	}

	WProperty* addGpioConfig() {
		byte index = this->numberOfGPIOs->getByte();
		char* pNumber = new char[6];
		//Config
		sprintf(pNumber, DEFAULT_GPIO_ID, index);
		WProperty* gpio = network->getSettings()->setByteArray(pNumber, DEFAULT_PROP_ARRAY);
		//Name
		sprintf(pNumber, DEFAULT_GPIO_NAME, index);
		network->getSettings()->setString(pNumber, "");
		//Title
		sprintf(pNumber, DEFAULT_GPIO_TITLE, index);
		network->getSettings()->setString(pNumber, "");
		return gpio;
	}


	void removeGpioConfig(byte index) {
		char* pNumber = new char[6];
		//Config
		sprintf(pNumber, DEFAULT_GPIO_ID, index);
		network->getSettings()->remove(pNumber);
		//Name
		sprintf(pNumber, DEFAULT_GPIO_NAME, index);
		network->getSettings()->remove(pNumber);
		//Title
		sprintf(pNumber, DEFAULT_GPIO_TITLE, index);
		network->getSettings()->remove(pNumber);
		char* p2 = new char[6];
		for (int i = index + 1; i < this->numberOfGPIOs->getByte(); i++) {
			//Config
			sprintf(pNumber, DEFAULT_GPIO_ID, i);
			sprintf(p2, DEFAULT_GPIO_ID, i - 1);
			network->getSettings()->getSetting(pNumber)->setId(p2);
			//Name
			sprintf(pNumber, DEFAULT_GPIO_NAME, i);
			sprintf(p2, DEFAULT_GPIO_NAME, i - 1);
			network->getSettings()->getSetting(pNumber)->setId(p2);
			//Title
			sprintf(pNumber, DEFAULT_GPIO_TITLE, i);
			sprintf(p2, DEFAULT_GPIO_TITLE, i - 1);
			network->getSettings()->getSetting(pNumber)->setId(p2);
		}
		this->numberOfGPIOs->setByte(this->numberOfGPIOs->getByte() - 1);
	}

	void updateEditingItem(AsyncWebServerRequest *request, Print* page) {
		bool exists = (request->hasParam("gpio", true));
		if (!exists) {
			page->print("New item");
			clearEditingItem();
		} else {
			String sIndex = request->getParam("gpio", true)->value();
			int index = atoi(sIndex.c_str());
			page->print("Edit item: ");
			page->print(sIndex);
			this->editingItem = getGpioConfig(index);
			this->editingName = getGpioName(index);
			this->editingTitle = getGpioTitle(index);
		}
	}

	void ensureEditingItem() {
		if (this->editingItem == nullptr) {
			this->editingItem = this->addGpioConfig();
			this->editingName = getGpioName(this->numberOfGPIOs->getByte());
			this->editingTitle = getGpioTitle(this->numberOfGPIOs->getByte());
			this->numberOfGPIOs->setByte(this->numberOfGPIOs->getByte() + 1);
		}
	}

	bool isGpioFree(byte gpio) {
		//exclude already used GPIOs
		if ((this->editingItem != nullptr) && (this->editingItem->getByteArrayValue(BYTE_GPIO) == gpio)) {
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

	bool addGpioChooserItem(Print* page, byte gpio, const char* title) {
		int aGpio = (this->editingItem != nullptr ? this->editingItem->getByteArrayValue(BYTE_GPIO) : 13);
		char* pNumber = new char[2];
		sprintf(pNumber, "%d", gpio);
		if (isGpioFree(gpio)) page->printf(HTTP_COMBOBOX_ITEM, pNumber, (aGpio == gpio ? HTTP_SELECTED : ""), title);
	}

  void addGpioChooser(Print* page, bool isOutput) {

		page->printf(HTTP_COMBOBOX_BEGIN, "GPIO:", "gp");
		#ifdef ESP8266
		addGpioChooserItem(page, 0, "0");
		if (isOutput) {
			addGpioChooserItem(page, 1, "1 (TX)");
			addGpioChooserItem(page, 2, "2");
		} else {
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
					noMax = max(noMax, (int) gName.substring(b + 1, gName.length()).toInt());
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
		//BYTE_SWITCH_LINKED_GPIO
		page->printf(HTTP_COMBOBOX_BEGIN, "Select output GPIO:", "lgp");
		byte linkedGpio = (this->editingItem != nullptr ? this->editingItem->getByteArrayValue(BYTE_LINKED_GPIO) : BYTE_NOT_LINKED_GPIO);
		//page->printf(HTTP_COMBOBOX_ITEM, "255", (BYTE_NOT_LINKED_GPIO == linkedGpio ? HTTP_SELECTED : ""), "None");
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
				page->printf(HTTP_COMBOBOX_ITEM, pNumber, (gGpio == linkedGpio ? HTTP_SELECTED : ""), iName.c_str());
			}
		}
		page->print(FPSTR(HTTP_COMBOBOX_END));
	}

	void printVisibility(Print* page, byte aProps, byte aType, String baseName) {
		bool mq = bitRead(aProps, BIT_CONFIG_MQTT);
		bool wt = bitRead(aProps, BIT_CONFIG_WEBTHING);
		page->printf(HTTP_TOGGLE_GROUP_STYLE, "ma", (mq ? HTTP_BLOCK : HTTP_NONE), "mb", HTTP_NONE);
		page->printf(HTTP_TOGGLE_GROUP_STYLE, "wa", (wt ? HTTP_BLOCK : HTTP_NONE), "wb", HTTP_NONE);
		//create MQTT property
		page->printf(HTTP_CHECKBOX_OPTION, "mq", "mq", (mq ? HTTP_CHECKED : ""), "tg()", "MQTT property");
		page->printf(HTTP_DIV_ID_BEGIN, "ma");
		//Property name
		page->printf(HTTP_TEXT_FIELD, "Property name:", "pn", "8", (this->editingName != nullptr ? this->editingName->c_str() : getNextName(aType, baseName).c_str()));
		//showAsWebthingDevice
		page->printf(HTTP_CHECKBOX_OPTION, "wt", "wt", (wt ? HTTP_CHECKED : ""), "tgw()", "Webthing property");
		page->printf(HTTP_DIV_ID_BEGIN, "wa");
		//Webthing title
		page->printf(HTTP_TEXT_FIELD, "Webthing title:", "wtt", "12", (this->editingTitle != nullptr ? this->editingTitle->c_str() : getNextName(aType, baseName).c_str()));
		page->print(FPSTR(HTTP_DIV_END));
		page->print(FPSTR(HTTP_DIV_END));
		page->printf(HTTP_TOGGLE_FUNCTION_SCRIPT, "tg()", "mq", "ma", "mb");
		page->printf(HTTP_TOGGLE_FUNCTION_SCRIPT, "tgw()", "wt", "wa", "wb");
	}

private:
	WProperty* numberOfGPIOs;
	WProperty* editingItem;
	WProperty* editingName;
	WProperty* editingTitle;

	void configureDevice() {
		//1. only outputs
		for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
			WProperty* gConfig = getGpioConfig(i);
			switch (gConfig->getByteArrayValue(BYTE_TYPE)) {
				case GPIO_TYPE_LED : {
					WLed* led = new WLed(gConfig->getByteArrayValue(BYTE_GPIO));
					this->addPin(led);
					bool mq = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_MQTT);
					bool wt = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_WEBTHING);
					if ((mq) || (wt)) {
						WProperty* ledProp = WProperty::createOnOffProperty(getGpioName(i)->c_str(), getGpioTitle(i)->c_str());
						ledProp->setVisibilityMqtt(mq);
						ledProp->setVisibilityWebthing(wt);
						this->addProperty(ledProp);
						led->setProperty(ledProp);
					}
					//Inverted
					led->setInverted(gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
					//Linkstate
					if (gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_LINKSTATE)) {
						network->setStatusLed(led, false);
					}
					break;
				}
				case GPIO_TYPE_RELAY : {
					WRelay* relay = new WRelay(gConfig->getByteArrayValue(BYTE_GPIO));
					this->addPin(relay);
					bool mq = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_MQTT);
					bool wt = gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_WEBTHING);
					if ((mq) || (wt)) {
						WProperty* relayProp = WProperty::createOnOffProperty(getGpioName(i)->c_str(), getGpioTitle(i)->c_str());
						relayProp->setVisibilityMqtt(mq);
						relayProp->setVisibilityWebthing(wt);
						this->addProperty(relayProp);
						relay->setProperty(relayProp);
					}
					//Inverted
					relay->setInverted(gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
					break;
				}

			}
		}
		//2. inputs
		for (byte i = 0; i < this->numberOfGPIOs->getByte(); i++) {
			WProperty* gConfig = getGpioConfig(i);
			WProperty* gName = getGpioName(i);
			byte gType = gConfig->getByteArrayValue(BYTE_TYPE);
			if ((gType == GPIO_TYPE_BUTTON) || (gType == GPIO_TYPE_SWITCH)) {

				byte bMode = (gType == GPIO_TYPE_SWITCH ? MODE_SWITCH : (gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_AP_LONGPRESS) ? MODE_BUTTON_LONG_PRESS : MODE_BUTTON));
				WSwitch* button = new WSwitch(gConfig->getByteArrayValue(BYTE_GPIO), bMode);
				//Inverted
				button->setInverted(gConfig->getByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED));
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
					//search property assigned to the GPIO
					for (byte b = 0; (!button->hasProperty()) && (b < this->numberOfGPIOs->getByte()); b++) {
						WProperty* gConfig2 = getGpioConfig(b);
						byte gType2 = gConfig2->getByteArrayValue(BYTE_TYPE);
						byte gGpio2 = gConfig2->getByteArrayValue(BYTE_GPIO);
						WProperty* gName2 = getGpioName(b);
						if ((!gName2->equalsString("")) && (isGpioAnOutput(gType2)) && (gGpio2 == linkedGpio)) {
							button->setProperty(this->getPropertyById(gName2->c_str()));
						}
					}
				} else if (!gName->equalsString("")) {
					//Only trigger via MQTT
					WProperty* triggerProperty = WProperty::createOnOffProperty(gName->c_str(), gName->c_str());
					triggerProperty->setVisibility(MQTT);
					this->addProperty(triggerProperty);
					button->setTriggerProperty(triggerProperty);
				}

			}
		}
	}

	void addGpioConfigItem(byte type, byte gpio, bool mqtt, bool webthing, bool inverted, String oName, String oTitle) {
		ensureEditingItem();
		this->editingItem->setByteArrayValue(BYTE_TYPE, type);
		this->editingItem->setByteArrayValue(BYTE_GPIO, gpio);
		this->editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_MQTT, mqtt);
		this->editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_WEBTHING, webthing);
		this->editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED, inverted);
		this->editingName->setString(oName.c_str());
		this->editingTitle->setString(oTitle.c_str());
	}

	void clearEditingItem() {
		this->editingItem = nullptr;
		this->editingName = nullptr;
		this->editingTitle = nullptr;
	}

	void addRelay(byte gpio, bool mqtt, bool webthing, bool inverted, String oName, String oTitle) {
		addGpioConfigItem(GPIO_TYPE_RELAY, gpio, mqtt, webthing, inverted, oName, oTitle);
		clearEditingItem();
	}

	void addLed(byte gpio, bool mqtt, bool webthing, bool inverted, String oName, String oTitle, bool linkState) {
		addGpioConfigItem(GPIO_TYPE_LED, gpio, mqtt, webthing, inverted, oName, oTitle);
		this->editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_LINKSTATE, linkState);
		clearEditingItem();
	}

	void addSwitch(byte gpio, byte linkedGpio, String oName) {
		addGpioConfigItem(GPIO_TYPE_SWITCH, gpio, false, false, false, oName, "");
		this->editingItem->setByteArrayValue(BYTE_LINKED_GPIO, linkedGpio);
		clearEditingItem();
	}

	void addButton(byte gpio, byte linkedGpio, String oName, bool inverted, bool switchApLongPress) {
		addGpioConfigItem(GPIO_TYPE_BUTTON, gpio, false, false, false, oName, "");
		this->editingItem->setByteArrayValue(BYTE_LINKED_GPIO, linkedGpio);
		this->editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED, inverted);
		this->editingItem->setByteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_AP_LONGPRESS, switchApLongPress);
		clearEditingItem();
	}

	/*static struct SwitchDevices TEMPLATES [7] =
	{
		//LED  RELAY		                 SWITCHES
	0	{ 4, {12, NO_PIN, NO_PIN, NO_PIN}, {13, NO_PIN, NO_PIN, NO_PIN}, {MODE_BUTTON_LONG_PRESS, NO_PIN, NO_PIN, NO_PIN}}, //Neo Coolcam
	1	{ 4, {13, NO_PIN, NO_PIN, NO_PIN}, {12, NO_PIN, NO_PIN, NO_PIN}, {MODE_SWITCH, NO_PIN, NO_PIN, NO_PIN}}, //Milos 1-fach
	2	{ 4, {13,     15, NO_PIN, NO_PIN}, {12,      5, NO_PIN, NO_PIN}, {MODE_SWITCH, MODE_SWITCH, NO_PIN, NO_PIN}}, //Milos 2-fach
	3	{13, {12, NO_PIN, NO_PIN, NO_PIN}, { 0,      4, NO_PIN, NO_PIN}, {MODE_BUTTON, MODE_SWITCH, NO_PIN, NO_PIN}}, //Sonoff Mini
	4	{13, {12, NO_PIN, NO_PIN, NO_PIN}, { 0, NO_PIN, NO_PIN, NO_PIN}, {MODE_BUTTON, NO_PIN, NO_PIN, NO_PIN}}, //Sonoff Basic
	5	{13, {12,      5,      4,     15}, { 0,      9,     10,     14}, {MODE_BUTTON, MODE_BUTTON, MODE_BUTTON, MODE_BUTTON}}, //Sonoff 4-channel
	6	{ 2, { 5, NO_PIN, NO_PIN, NO_PIN}, { 0, NO_PIN, NO_PIN, NO_PIN}, {MODE_BUTTON, NO_PIN, NO_PIN, NO_PIN}} //Wemos: Relay at D1, Switch at D3
	};*/
	void configureTemplate(byte templateIndex) {
		//clear all GPIOs
		for (int i = this->numberOfGPIOs->getByte() -1; i >= 0; i--) {
			this->removeGpioConfig(i);
		}
		switch (templateIndex) {
			case 0: {
				//Neo Coolcam
				this->addLed(   4, false, false, false, "led",   "", true);
				this->addRelay(12,  true,  true, false,  "on", "on");
				this->addButton(13, 12, "", false, true);
				break;
			}
			case 1: {
				//Milos 1-fach
				this->addLed(   4, false, false, false, "led",   "", true);
				this->addRelay(13,  true,  true, false,  "on", "on");
				this->addSwitch(12, 13, "");
				break;
			}
			case 2: {
				//Milos 2-fach
				this->addLed(   4, false, false, false, "led",   "", true);
				this->addRelay(13,  true,  true, false,  "on", "on");
				this->addRelay(15,  true,  true, false,  "on_2", "on_2");
				this->addSwitch(12, 13, "");
				this->addSwitch( 5, 15, "");
				break;
			}
			case 3: {
				//Sonoff mini + Switch
				this->addLed(  13, false, false, false, "led",   "", true);
				this->addRelay(12,  true,  true, false,  "on", "on");
				this->addButton(0, 12, "", true, true);
				this->addSwitch(4, 12, "");
				break;
			}
			case 4: {
				//Sonoff mini + Button
				this->addLed(  13, false, false, false, "led",   "", true);
				this->addRelay(12,  true,  true, false,  "on", "on");
				this->addButton(0, 12, "", true, true);
				this->addButton(4, 12, "", true, false);
				break;
			}
			case 5: {
				//Sonoff Basic
				this->addLed(  13, false, false, false, "led",   "", true);
				this->addRelay(12,  true,  true, false,  "on", "on");
				this->addButton(0, 12, "", false, true);
				break;
			}
			case 6: {
				//Sonoff 4-channel
				this->addLed(  13, false, false, false, "led",   "", true);
				this->addRelay(12,  true,  true, false,  "on", "on");
				this->addRelay( 5,  true,  true, false,  "on_2", "on_2");
				this->addRelay( 4,  true,  true, false,  "on_3", "on_3");
				this->addRelay(15,  true,  true, false,  "on_4", "on_4");
				this->addButton( 0, 12, "", false, true);
				this->addButton( 9,  5, "", false, false);
				this->addButton(10,  4, "", false, false);
				this->addButton(14, 15, "", false, false);
				break;
			}
			case 7: {
				//Wemos: Relay at D1, Switch at D3
				this->addLed(  2, false, false, false, "led",   "", true);
				this->addRelay(5,  true,  true, false,  "on", "on");
				this->addButton(0, 5, "", false, true);
				break;
			}
		}


	}

};

#endif
