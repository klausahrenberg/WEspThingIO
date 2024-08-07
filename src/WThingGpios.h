#ifndef W_THING_GIOS_H
#define W_THING_GIOS_H

#define MAX_GPIOS 12
#define GPIO_DEFAULT_ID "^g.%d"
#define GPIO_DEFAULT_ID_SUB GPIO_DEFAULT_ID ".%d"
const byte* DEFAULT_PROP_ARRAY = (const byte[]){0, 0, 0, 0};

#define MAX_PROP_BYTES 4
#define GPIO_TYPE_UNKNOWN 0xFF

enum WGpioType {
  //Mode 0..7

  //Merge 8..15

  //Output 16..127
  //gpio

  //sca, scl

  //din, sclk, cs

  //Input 128..254




  GPIO_TYPE_LED = 0, GPIO_TYPE_RELAY = 5, GPIO_TYPE_BUTTON, 
  GPIO_TYPE_SWITCH, GPIO_TYPE_MODE, GPIO_TYPE_RGB_LED, 
  GPIO_TYPE_MERGE, GPIO_TYPE_TEMP_SENSOR, GPIO_TYPE_DIMMER
};

const static char S_GPIO_TYPE_LED[] PROGMEM = "led";
const static char S_GPIO_TYPE_RELAY[] PROGMEM = "relay";
const static char S_GPIO_TYPE_BUTTON[] PROGMEM = "button";
const static char S_GPIO_TYPE_SWITCH[] PROGMEM = "switch";
const static char S_GPIO_TYPE_MODE[] PROGMEM = "mode";
const static char S_GPIO_TYPE_RGB_LED[] PROGMEM = "rgb";
const static char S_GPIO_TYPE_MERGE[] PROGMEM = "merge";
const static char S_GPIO_TYPE_TEMP_SENSOR[] PROGMEM = "temp";
const static char S_GPIO_TYPE_DIMMER[] PROGMEM = "dimmer";
const char* const S_GPIO_TYPE[] PROGMEM = { S_GPIO_TYPE_LED, S_GPIO_TYPE_RELAY, S_GPIO_TYPE_BUTTON, 
                                             S_GPIO_TYPE_SWITCH, S_GPIO_TYPE_MODE, S_GPIO_TYPE_RGB_LED, 
                                             S_GPIO_TYPE_MERGE, S_GPIO_TYPE_TEMP_SENSOR, S_GPIO_TYPE_DIMMER };

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

class WThingGpios : public IWIterable<WValue> {
 public: 
  WThingGpios() {
    _numberOfGPIOs = SETTINGS->setByte(TG_NUMBER_OF_GPIOS, 0, MAX_GPIOS);
    
    for (byte index = 0; index < _numberOfGPIOs->asByte(); index++) {
      WValue pNumber = WValue::ofPattern(GPIO_DEFAULT_ID, index);
      Serial.println(pNumber.asString());
      WValue* gType = SETTINGS->setByteArray(pNumber.asString(), 2, (const byte[]){GPIO_TYPE_UNKNOWN, 0});
      
      IWStorable* storable = nullptr;
      switch (gType->asByteArray()[0]) {
        case GPIO_TYPE_LED : {
          Serial.println("led gefunden");
          storable = new WLed(NO_PIN);
          break;
        }  
        default :
          Serial.print("unknown: ");
          Serial.println(gType->asByteArray()[0]);
      }

    }  








    /*_numberOfGPIOs = SETTINGS->setByte(TG_NUMBER_OF_GPIOS, 0, MAX_GPIOS);
    
    byte nog = _numberOfGPIOs->asByte();
    _numberOfGPIOs->asByte(0);
    for (byte i = 0; i < nog; i++) {
      _add(GPIO_TYPE_UNKNOWN);      
    }

    //_addLed(0, 2, false, false, false, false, "led", "", true);

    Serial.print("numberOfGpios ");
    Serial.println(_numberOfGPIOs->asByte());

    _add(GPIO_TYPE_LED);
    _add(GPIO_TYPE_BUTTON);

    Serial.print("numberOfGpis ");
    Serial.println(_numberOfGPIOs->asByte());*/
    //SETTINGS->save();
  }

  typedef std::function<void(int, WValue*, const char*)> TOnIteration;
  virtual void forEach(TOnIteration consumer) {
    if (consumer) {
      for (int i = 0; i < _numberOfGPIOs->asByte(); i++) consumer(i, getGpioConfig(i), nullptr);
    }
  }

  WValue* getGroupedGpioByName(const char* name) {
    return getGroupedGpioBySubString(name, FIRST_NAME);
  }

  WValue* getGroupedGpioBySubString(const char* name, byte subStringIndex) {
    for (byte i = 0; i < _numberOfGPIOs->asByte(); i++) {
      WValue* gConfig = getGpioConfig(i);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      if (_isGpioGrouped(gType)) {
        const char* gName = getSubString(i, subStringIndex);
        if ((gName != nullptr) && (strcmp(gName, name) == 0)) {
          return gConfig;
        }
      }
    }
    return nullptr;
  }

  WValue* numberOfGPIOs() { return _numberOfGPIOs; }

  WValue* getGpioConfig(byte index) {
    return SETTINGS->getById(WValue::ofPattern(GPIO_DEFAULT_ID, index).asString());    
  }

  const char* getSubString(byte index, byte subIndex) {
    WValue* p = _getSubProperty(index, subIndex);
    return (p != nullptr ? p->asString() : nullptr);
  }

  const char* getGpioDisplayName(byte gType) {    
    switch (gType) {
      case GPIO_TYPE_LED: return S_GPIO_TYPE_LED;
      case GPIO_TYPE_RELAY: return "Relay";
      case GPIO_TYPE_DIMMER: return "Dimmer";
      case GPIO_TYPE_BUTTON: return "Button";
      case GPIO_TYPE_SWITCH: return "Switch";
      case GPIO_TYPE_MODE: return "On/Mode";
      case GPIO_TYPE_MERGE: return "Merge";
      case GPIO_TYPE_RGB_LED: return "RGB";
      case GPIO_TYPE_TEMP_SENSOR: return "TempSens";
      default: return "n.a.";
    }
  }

  void toJson(Print* stream) {    
    WJson json = WJson(stream);
    json.beginArray();
    forEach([this, &json](int index, WValue* gConfig, const char* id) {

      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      
      json.beginObject();
      
      json.propertyString(WC_TYPE, S_GPIO_TYPE[gType], nullptr);
      if (_isGpioUsingGPIO(gType)) {
        json.propertyByte(PSTR("gpio"), gConfig->byteArrayValue(BYTE_GPIO));
      }
      if (_isGpioUsingSCL(gType)) {
        json.propertyByte(PSTR("scl"), gConfig->byteArrayValue(BYTE_SCL));
      }

      for (int i = 0; i < _getNumberOfChars(gType); i++) {
        json.propertyString(WValue::ofPattern("s_%s", "ho").asString(), "t");// _getSubProperty(index, i)->asString());
      }
      
      
      json.endObject();
      //json.separator();
    });
    json.endArray();    
  }

 protected:
  WValue* _numberOfGPIOs;
  //byte _numberOfSettings;

  WValue* _getSubProperty(byte index, byte subIndex) {
    WValue pNumber = WValue::ofPattern(GPIO_DEFAULT_ID_SUB, index, subIndex);
    return SETTINGS->getById(pNumber.c_str());
  }

  WValue* _getGroupedGpioByModeName(const char* name) {
    return getGroupedGpioBySubString(name, SECOND_NAME);
  }

  WValue* _add(byte gType) {
    byte index = _numberOfGPIOs->asByte();

    WValue pNumber = WValue::ofPattern(GPIO_DEFAULT_ID, index);
    Serial.println(pNumber.asString());
    WValue* gConfig = new WValue(4, DEFAULT_PROP_ARRAY);
    SETTINGS->add(gConfig, pNumber.asString());
    //delete pNumber;
    //type;
    if (gType == GPIO_TYPE_UNKNOWN) {
      gType = gConfig->byteArrayValue(BYTE_TYPE);
    } else {
      gConfig->byteArrayValue(BYTE_TYPE, gType);
    }
    
    for (byte i = 0; i < _getNumberOfChars(gType); i++) {
      WValue subNumber = WValue::ofPattern(GPIO_DEFAULT_ID_SUB, index, i);
      Serial.println(subNumber.asString());      
      SETTINGS->add(new WValue(STRING), subNumber.asString());      
      //delete subNumber;
    }
    _numberOfGPIOs->asByte(_numberOfGPIOs->asByte() + 1);

    return gConfig;
  }

  void _removeGpioConfig(byte index) {
    WValue* gConfig = getGpioConfig(index);
    byte gType = gConfig->byteArrayValue(BYTE_TYPE);
    String pNumber = GPIO_DEFAULT_ID;
    pNumber.concat(index);
    // Config
    SETTINGS->remove(pNumber.c_str());
    //_numberOfSettings--;
    pNumber.concat(".");
    for (byte i = 0; i < _getNumberOfChars(gType); i++) {
      String subNumber = pNumber;
      subNumber.concat(i);
      SETTINGS->remove(subNumber.c_str());      
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
    /*if ((_editingItem != nullptr) && (_editingItem->byteArrayValue(BYTE_GPIO) == gpio)) {
      return true;
    } else {*/
      for (byte i = 0; i < _numberOfGPIOs->asByte(); i++) {
        WValue* gConfig = getGpioConfig(i);
        byte gType = gConfig->byteArrayValue(BYTE_TYPE);
        if (((_isGpioUsingGPIO(gType)) && (gConfig->byteArrayValue(BYTE_GPIO) == gpio)) ||
            ((_isGpioUsingSCL(gType)) && (gConfig->byteArrayValue(BYTE_SCL) == gpio))) {
          return false;
        }
      }
    //}
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

  WValue* _setSubString(WValue* gpioConfig, byte subIndex, String value) {
    /*String pNumber = gpioConfig->title();
    pNumber.concat(".");
    pNumber.concat(subIndex);
    return SETTINGS->setString(pNumber.c_str(), value.c_str());*/
    return nullptr;
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
      WValue* gConfig = getGpioConfig(i);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      byte aProps = gConfig->byteArrayValue(BYTE_CONFIG);
      bool gr = bitRead(aProps, BIT_CONFIG_PROPERTY_GROUPED);
      if (aType == gType) {
        int b = -1;
        String gName = "";
        if (!gr) {
          const char* gN = getSubString(i, subIndex);
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

  

};

#endif