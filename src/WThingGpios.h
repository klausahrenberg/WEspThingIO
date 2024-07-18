#ifndef W_THING_GIOS_H
#define W_THING_GIOS_H

#define MAX_GPIOS 12
#define GPIO_DEFAULT_ID "^g_"
const byte* DEFAULT_PROP_ARRAY = (const byte[]){0, 0, 0, 0};

#define MAX_PROP_BYTES 4
#define GPIO_TYPE_LED 0
#define GPIO_TYPE_RELAY 1
#define GPIO_TYPE_BUTTON 2
#define GPIO_TYPE_SWITCH 3
#define GPIO_TYPE_MODE 4
#define GPIO_TYPE_RGB_LED 5
#define GPIO_TYPE_MERGE 6
#define GPIO_TYPE_TEMP_SENSOR 7
#define GPIO_TYPE_DIMMER 8
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
#define BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE BIT_CONFIG_LED_INVERTED
#define BIT_CONFIG_NO_OF_LEDS BIT_CONFIG_LED_INVERTED
#define BIT_CONFIG_TEMP_SENSOR_TYPE BIT_CONFIG_LED_INVERTED
#define BIT_CONFIG_LED_LINKSTATE 4
#define BIT_CONFIG_SWITCH_AP_LONGPRESS BIT_CONFIG_LED_LINKSTATE
#define BIT_CONFIG_SWITCH_DIRECTLY 5

const static char TG_NUMBER_OF_GPIOS[] PROGMEM = "numberOfGPIOs";

class WThingGpios {
 public: 
  WThingGpios() {
    _editingItem = nullptr;
    _editingIndex = NO_INDEX;
    _numberOfGPIOs = SETTINGS->setByte(TG_NUMBER_OF_GPIOS, 0, MAX_GPIOS);
    _numberOfGPIOs->visibility(NONE);
    //this->addProperty(_numberOfGPIOs,TG_NUMBER_OF_GPIOS );
    _numberOfSettings = SETTINGS->size();
    byte nog = _numberOfGPIOs->asByte();
    _numberOfGPIOs->asByte(0);
    for (byte i = 0; i < nog; i++) {
      _addGpioConfig(GPIO_TYPE_UNKNOWN);
      _numberOfGPIOs->asByte(i + 1);
    }
  }

  char* getSubString(WProperty* gpioConfig, byte subIndex) {
    WProperty* p = _getSubProperty(gpioConfig, subIndex);
    return (p != nullptr ? p->c_str() : nullptr);
  }

  WProperty* getGroupedGpioByName(const char* name) {
    return getGroupedGpioBySubString(name, FIRST_NAME);
  }

  WProperty* getGroupedGpioBySubString(const char* name, byte subStringIndex) {
    for (byte i = 0; i < _numberOfGPIOs->asByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      if (_isGpioGrouped(gType)) {
        char* gName = getSubString(gConfig, subStringIndex);
        if ((gName != nullptr) && (strcmp(gName, name) == 0)) {
          return gConfig;
        }
      }
    }
    return nullptr;
  }

  WProperty* numberOfGPIOs() { return _numberOfGPIOs; }

  WProperty* getGpioConfig(byte index) {
    String pNumber = GPIO_DEFAULT_ID;
    pNumber.concat(index);
    return SETTINGS->getSetting(pNumber.c_str());
  }

 protected:
  WProperty* _numberOfGPIOs;
  byte _numberOfSettings;
  WProperty* _editingItem;
  byte _editingIndex;

  WProperty* _getGroupedGpioByModeName(const char* name) {
    return getGroupedGpioBySubString(name, SECOND_NAME);
  }

  WProperty* _addGpioConfig(byte gType) {
    byte index = _numberOfGPIOs->asByte();
    String pNumber = GPIO_DEFAULT_ID;
    pNumber.concat(index);
    // Config
    // WProperty* gConfig = network()->settings()->setByteArray(pNumber.c_str(),
    // DEFAULT_PROP_ARRAY);
    WProperty* gConfig = new WProperty(pNumber.c_str(), BYTE_ARRAY, "");
    gConfig->asByteArray(4, DEFAULT_PROP_ARRAY);
    SETTINGS->insert(gConfig, _numberOfSettings, pNumber.c_str());
    _numberOfSettings++;
    if (gType == GPIO_TYPE_UNKNOWN) {
      gType = gConfig->byteArrayValue(BYTE_TYPE);
    } else {
      gConfig->byteArrayValue(BYTE_TYPE, gType);
    }
    pNumber.concat(".");
    for (byte i = 0; i < _getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);
      // network()->settings()->setString(subNumber.c_str(), "");
      WProperty* sp = WProps::createStringProperty();
      SETTINGS->insert(sp, _numberOfSettings, subNumber.c_str());
      _numberOfSettings++;
    }
    return gConfig;
  }

  void _removeGpioConfig(byte index) {
    WProperty* gConfig = getGpioConfig(index);
    byte gType = gConfig->byteArrayValue(BYTE_TYPE);
    String pNumber = GPIO_DEFAULT_ID;
    pNumber.concat(index);
    // Config
    SETTINGS->remove(pNumber.c_str());
    _numberOfSettings--;
    pNumber.concat(".");
    for (byte i = 0; i < _getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);
      SETTINGS->remove(subNumber.c_str());
      _numberOfSettings--;
    }
    // Move next gpios 1 place forward

    for (int i = index + 1; i < _numberOfGPIOs->asByte(); i++) {
      gConfig = getGpioConfig(i);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      // Config
      pNumber = GPIO_DEFAULT_ID;
      pNumber.concat(i);
      String p2 = GPIO_DEFAULT_ID;
      p2.concat(i - 1);
      SETTINGS->changeId(pNumber.c_str(), p2.c_str());
      pNumber.concat(".");
      p2.concat(".");
      for (byte b = 0; b < _getNumberOfChars(gType); b++) {
        String subNumber = pNumber;
        subNumber.concat(b);
        String subP2 = p2;
        subP2.concat(b);
        SETTINGS->changeId(subNumber.c_str(), subP2.c_str());
      }
    }
    _numberOfGPIOs->asByte(_numberOfGPIOs->asByte() - 1);
  }
   
  bool _isGpioFree(byte gpio) {
    // exclude already used GPIOs
    if ((_editingItem != nullptr) && (_editingItem->byteArrayValue(BYTE_GPIO) == gpio)) {
      return true;
    } else {
      for (byte i = 0; i < _numberOfGPIOs->asByte(); i++) {
        WProperty* gConfig = getGpioConfig(i);
        byte gType = gConfig->byteArrayValue(BYTE_TYPE);
        if (((_isGpioUsingGPIO(gType)) && (gConfig->byteArrayValue(BYTE_GPIO) == gpio)) ||
            ((_isGpioUsingSCL(gType)) && (gConfig->byteArrayValue(BYTE_SCL) == gpio))) {
          return false;
        }
      }
    }
    return true;
  }

  bool _isGpioAnOutput(byte gType) {
    return ((gType == GPIO_TYPE_LED) || (gType == GPIO_TYPE_RELAY) ||
            (gType == GPIO_TYPE_DIMMER) || (gType == GPIO_TYPE_RGB_LED));
  }

  bool _isGpioAnInput(byte gType) {
    return ((gType == GPIO_TYPE_BUTTON) || (gType == GPIO_TYPE_SWITCH) ||
            (gType == GPIO_TYPE_TEMP_SENSOR));
  }

  bool _isGpioGrouped(byte gType) {
    return ((gType == GPIO_TYPE_MODE) || (gType == GPIO_TYPE_MERGE));
  }

  bool _isGpioUsingGPIO(byte gType) {
    return (!_isGpioGrouped(gType));
  }

  bool _isGpioUsingSCL(byte gType) {
    return (gType == GPIO_TYPE_TEMP_SENSOR);
  }

  WProperty* _setSubString(WProperty* gpioConfig, byte subIndex, String value) {
    String pNumber = gpioConfig->title();
    pNumber.concat(".");
    pNumber.concat(subIndex);
    return SETTINGS->setString(pNumber.c_str(), value.c_str());
  }

  WProperty* _getSubProperty(WProperty* gpioConfig, byte subIndex) {
    if (gpioConfig != nullptr) {
      String pNumber = gpioConfig->title();
      pNumber.concat(".");
      pNumber.concat(subIndex);
      return SETTINGS->getSetting(pNumber.c_str());
    } else {
      return nullptr;
    }
  }

  byte _getNumberOfChars(byte gType) {
    switch (gType) {
      case GPIO_TYPE_MERGE:
        return 3;
      case GPIO_TYPE_MODE:
        return 4;
      case GPIO_TYPE_LED:
        return 2;
      case GPIO_TYPE_RELAY:
        return 2;
      case GPIO_TYPE_DIMMER:
        return 4;
      case GPIO_TYPE_RGB_LED:
        return 2;
      case GPIO_TYPE_TEMP_SENSOR:
        return 4;
      case GPIO_TYPE_SWITCH:
        return 1;
      case GPIO_TYPE_BUTTON:
        return 1;
      default:
        return 0;
    }
  }

  String _getNextName(byte aType, byte subIndex, String baseName) {
    int noMax = 0;
    for (byte i = 0; i < _numberOfGPIOs->asByte(); i++) {
      WProperty* gConfig = getGpioConfig(i);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      byte aProps = gConfig->byteArrayValue(BYTE_CONFIG);
      bool gr = bitRead(aProps, BIT_CONFIG_PROPERTY_GROUPED);
      if (aType == gType) {
        int b = -1;
        String gName = "";
        if (!gr) {
          char* gN = getSubString(gConfig, subIndex);
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

  void _addGpioConfigItem(byte type, byte gpio, bool grouped, bool mqtt, bool webthing, bool inverted) {
    _ensureEditingItem(type);
    //_editingItem->setByteArrayValue(BYTE_TYPE, type);
    _editingItem->byteArrayValue(BYTE_GPIO, gpio);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_GROUPED, grouped);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_MQTT, mqtt);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_PROPERTY_WEBTHING, webthing);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED, inverted);

    String pNumber = _editingItem->title();
    pNumber.concat(".");
    byte gType = _editingItem->byteArrayValue(BYTE_TYPE);
    for (byte i = 0; i < _getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);
      SETTINGS->setString(subNumber.c_str(), "");
    }
  }

  void _clearEditingItem() {
    _editingItem = nullptr;
    _editingIndex = NO_INDEX;
  }

  void _addRelay(byte gpio, bool gr, bool mqtt, bool webthing, bool inverted, String oName, String oTitle) {
    _addGpioConfigItem(GPIO_TYPE_RELAY, gpio, gr, mqtt, webthing, inverted);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _setSubString(_editingItem, FIRST_TITLE, oTitle);
    _clearEditingItem();
  }

  void _addDimmer(byte gpio, bool webthing, String oName, String oTitle, String lName, String lTitle) {
    _addGpioConfigItem(GPIO_TYPE_DIMMER, gpio, false, true, webthing, false);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _setSubString(_editingItem, FIRST_TITLE, oTitle);
    _setSubString(_editingItem, SECOND_NAME, lName);
    _setSubString(_editingItem, SECOND_TITLE, lTitle);
    _clearEditingItem();
  }

  void _addLed(byte gpio, bool gr, bool mqtt, bool webthing, bool inverted, String oName, String oTitle, bool linkState) {
    _addGpioConfigItem(GPIO_TYPE_LED, gpio, gr, mqtt, webthing, inverted);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _setSubString(_editingItem, FIRST_TITLE, oTitle);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_LINKSTATE, linkState);
    _clearEditingItem();
  }

  void _addRgbLed(byte gpio, bool gr, bool mqtt, bool webthing, byte noOfLeds, String oName, String oTitle, bool groupInMode) {
    _addGpioConfigItem(GPIO_TYPE_RGB_LED, gpio, gr, mqtt, webthing, false);
    _editingItem->byteArrayValue(BYTE_NO_OF_LEDS, noOfLeds);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _setSubString(_editingItem, FIRST_TITLE, oTitle);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_RGB_GROUPED_PROGRAMS_IN_MODE, groupInMode);
    _clearEditingItem();
  }

  void _addTempSensor(byte sda, byte scl, bool mqtt, bool webthing, bool isHtu21, String tName, String tTitle, String hName, String hTitle) {
    _addGpioConfigItem(GPIO_TYPE_TEMP_SENSOR, sda, false, mqtt, webthing, false);
    _editingItem->byteArrayValue(BYTE_SCL, scl);
    _setSubString(_editingItem, FIRST_NAME, tName);
    _setSubString(_editingItem, FIRST_TITLE, tTitle);
    _setSubString(_editingItem, SECOND_NAME, hName);
    _setSubString(_editingItem, SECOND_TITLE, hTitle);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_TEMP_SENSOR_TYPE, isHtu21);
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

  void _addMergedProperty(bool gr, bool mqtt, bool webthing, bool inverted, String propertyName, String propertyTitle, String groupedName) {
    _addGpioConfigItem(GPIO_TYPE_MERGE, BYTE_NO_GPIO, gr, mqtt, webthing, inverted);
    _setSubString(_editingItem, FIRST_NAME, propertyName);
    _setSubString(_editingItem, FIRST_TITLE, propertyTitle);
    _setSubString(_editingItem, SECOND_NAME, groupedName);
    _clearEditingItem();
  }

  void _addSwitch(byte gpio, bool switchDirect, String oName) {
    _addGpioConfigItem(GPIO_TYPE_SWITCH, gpio, false, false, false, false);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_DIRECTLY, switchDirect);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _clearEditingItem();
  }

  void _addButton(byte gpio, bool switchDirect, String oName, bool inverted, bool switchApLongPress) {
    _addGpioConfigItem(GPIO_TYPE_BUTTON, gpio, false, false, false, false);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_DIRECTLY, switchDirect);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_LED_INVERTED, inverted);
    _editingItem->byteArrayBitValue(BYTE_CONFIG, BIT_CONFIG_SWITCH_AP_LONGPRESS, switchApLongPress);
    _setSubString(_editingItem, FIRST_NAME, oName);
    _clearEditingItem();
  }

  void _ensureEditingItem(byte gType) {
    if (_editingIndex == NO_INDEX) {
      _editingIndex = _numberOfGPIOs->asByte();
      _editingItem = _addGpioConfig(gType);
      _numberOfGPIOs->asByte(_editingIndex + 1);
    }
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
      _editingItem = getGpioConfig(index);
      return false;
    }
  }


};

#endif