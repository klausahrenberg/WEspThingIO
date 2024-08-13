#ifndef W_ESP_THING_IO_H
#define W_ESP_THING_IO_H

#include "WDevice.h"

#define DEVICE_ID "switch"
#define MAX_GPIOS 12
#define GPIO_DEFAULT_ID "^g.%d"
#define GPIO_DEFAULT_ID_SUB GPIO_DEFAULT_ID ".%d"
#define GPIO_TYPE_COUNT 9

const static char TG_NUMBER_OF_GPIOS[] PROGMEM = "numberOfGPIOs";

enum WGpioType {
  //Mode 0..7
  //Merge 8..15
  //Output 16..127
  //Input 128..254
  GPIO_TYPE_LED = 0, GPIO_TYPE_RELAY = 5, GPIO_TYPE_BUTTON, 
  GPIO_TYPE_SWITCH, GPIO_TYPE_MODE, GPIO_TYPE_RGB_LED, 
  GPIO_TYPE_MERGE, GPIO_TYPE_TEMP_SENSOR, GPIO_TYPE_DIMMER,

  GPIO_TYPE_UNKNOWN = 0xFF
};

const char S_GPIO_TYPE_LED[] PROGMEM = "led";
const char S_GPIO_TYPE_RELAY[] PROGMEM = "relay";
const char S_GPIO_TYPE_BUTTON[] PROGMEM = "button";
const char S_GPIO_TYPE_SWITCH[] PROGMEM = "switch";
const char S_GPIO_TYPE_MODE[] PROGMEM = "mode";
const char S_GPIO_TYPE_RGB_LED[] PROGMEM = "rgb";
const char S_GPIO_TYPE_MERGE[] PROGMEM = "merge";
const char S_GPIO_TYPE_TEMP_SENSOR[] PROGMEM = "temp";
const char S_GPIO_TYPE_DIMMER[] PROGMEM = "dimmer";
const char* const S_GPIO_TYPE[] PROGMEM = { S_GPIO_TYPE_LED, S_GPIO_TYPE_RELAY, S_GPIO_TYPE_BUTTON, 
                                             S_GPIO_TYPE_SWITCH, S_GPIO_TYPE_MODE, S_GPIO_TYPE_RGB_LED, 
                                             S_GPIO_TYPE_MERGE, S_GPIO_TYPE_TEMP_SENSOR, S_GPIO_TYPE_DIMMER };

struct WThing { 
  WGpioType type;
  byte config;
  //bool _isNull = true;
  //const char* _toString = nullptr;
  union {
    WLed* asLed;
    //WRelay* asRelay;    
  };
};

class WThingIO : public WDevice, public IWIterable<WThing> {
 public:
  WThingIO(WNetwork *network)
      : WDevice(network, DEVICE_ID, DEVICE_ID, DEVICE_TYPE_ON_OFF_SWITCH, DEVICE_TYPE_LIGHT) {    
    _numberOfGPIOs = SETTINGS->setByte(TG_NUMBER_OF_GPIOS, 0, MAX_GPIOS);
    _items = new WList<WThing>();
    //Read from EEPROM
    for (byte index = 0; index < _numberOfGPIOs->asByte(); index++) {
      WValue pNumber = WValue::ofPattern(GPIO_DEFAULT_ID, index);
      Serial.println(pNumber.asString());
      WValue* gType = SETTINGS->setByteArray(pNumber.asString(), 2, (const byte[]){GPIO_TYPE_UNKNOWN, 0});
      
      WThing* thing = new WThing();
      thing->type = (WGpioType) gType->asByteArray()[0];
      thing->config = gType->asByteArray()[1];
      _items->add(thing);

      IWStorable* storable = nullptr;
      switch (thing->type) {
        case GPIO_TYPE_LED : {
          Serial.println("led gefunden");
          thing->asLed = new WLed(NO_PIN);          
          this->addOutput(thing->asLed);
          storable = thing->asLed;
          break;
        }  
        default :
          Serial.print("unknown: ");
          Serial.println();
      }
      storable->loadFromStore();            
    }         
    
    //Configure items
    _items->forEach([this](int index, WThing* thing, const char* id) {
      bool mq = true;// bitRead(thing->config, BIT_CONFIG_PROPERTY_MQTT);
      bool wt = true;// bitRead(thing->config, BIT_CONFIG_PROPERTY_WEBTHING);
      switch (thing->type) {
        case GPIO_TYPE_LED : {                    
          if (thing->asLed->linkState()) { this->network()->setStatusLed(thing->asLed, false); }
          break;
        }                    
      }      
      if (_isOutput(thing->type)) {
        WOutput* output = (WOutput*) thing->asLed;
        const char* tid = "test";//thing->asLed->id();
        WThing* linkedThing = this->_items->getById(tid);
        if ((tid != nullptr) && (strcmp_P(tid, "") != 0) && (linkedThing == nullptr)) {          
          if ((wt) || (mq)) {
            LOG->debug("add default property '%s'", tid);
            WProperty* prop = WProps::createOnOffProperty(tid);
            prop->visibilityMqtt(mq);
            prop->visibilityWebthing(wt);
            output->on(prop);
            this->addProperty(prop, tid);
          }
        }
      }
    });
  }

  virtual WFormResponse* loadFromJson(WList<WValue>* list) {          
    LOG->debug("list items count: %d", list->size());
    //_gpios->clear();
    list->forEach([this](int index, WValue* value, const char* key) {
      LOG->debug("key '%s' / value '%s'", key, value->toString());
      if (value->type() == LIST) { 
        WGpioType gType = _gpioTypeOf(value->asList()->getById(WC_TYPE));
        if (gType != GPIO_TYPE_UNKNOWN) {
          WThing* thing = new WThing();
          thing->type = gType;
          thing->config = 0x00;
          bitWrite(thing->config, BIT_CONFIG_PROPERTY_WEBTHING, (value->asList()->getById(WC_WEBTHING)->asBool()));
          bitWrite(thing->config, BIT_CONFIG_PROPERTY_MQTT, (value->asList()->getById(WC_MQTT)->asBool()));
          _items->add(thing);

          LOG->debug("type is %s", S_GPIO_TYPE[gType]);

          value->asList()->forEach([](int index, WValue* subValue, const char* subId) {
            LOG->debug(" >> subkey '%s' / subvalue '%s'", subId, subValue->toString());

          });
        } else {
          LOG->debug("no type found");
        }
        Serial.println("c");
      }
    });
    //_addLed(byte gpio, bool gr, bool mqtt, bool webthing, bool inverted, String oName, String oTitle, bool linkState)
    return new WFormResponse(FO_RESTART, PSTR("submit ThingPage"));
  }  

  typedef std::function<void(int, WThing*, const char*)> TOnIteration;
  virtual void forEach(TOnIteration consumer) {
    if (consumer) {
      for (int i = 0; i < _numberOfGPIOs->asByte(); i++) consumer(i, _items->get(i), nullptr);
    }
  }

  void toJson(Print* stream) {    
    WJson json = WJson(stream);
    json.beginArray();
    _items->forEach([this, &json](int index, WThing* thing, const char* id) {

      /*byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      
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
      
      
      json.endObject();*/
      //json.separator();
    });
    json.endArray();    
  }

  WValue* numberOfGPIOs() { return _numberOfGPIOs; }

 protected:
  WValue* _numberOfGPIOs;
  WList<WThing>* _items;

  bool _isOutput(byte type) {
    //tbi
    return true;
  }

  WGpioType _gpioTypeOf(WValue* jType) {    
    if (jType != nullptr) {
      for (byte b = 0; b < GPIO_TYPE_COUNT; b++) {      
        if (jType->equalsString(S_GPIO_TYPE[b])) {        
          return (WGpioType) b;
        }
      }
    }
    return GPIO_TYPE_UNKNOWN;
  }
  
};

#endif