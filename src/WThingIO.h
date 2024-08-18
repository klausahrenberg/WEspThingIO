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
  //Mode  
  GPIO_TYPE_MODE,
  //Merge
  GPIO_TYPE_MERGE,
  //Outputs
  GPIO_TYPE_LED, GPIO_TYPE_RELAY,
  GPIO_TYPE_RGB_LED, GPIO_TYPE_DIMMER,
  //Inputs
  GPIO_TYPE_BUTTON, GPIO_TYPE_SWITCH,  
  GPIO_TYPE_TEMP_SENSOR,
  //NONE
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
const char* const S_GPIO_TYPE[] PROGMEM = { S_GPIO_TYPE_MODE, S_GPIO_TYPE_MERGE,
                                            S_GPIO_TYPE_LED, S_GPIO_TYPE_RELAY, S_GPIO_TYPE_RGB_LED, S_GPIO_TYPE_DIMMER, 
                                            S_GPIO_TYPE_BUTTON, S_GPIO_TYPE_SWITCH, S_GPIO_TYPE_TEMP_SENSOR };

struct WThing { 
  WGpioType type;
  byte config;
  //bool _isNull = true;
  //const char* _toString = nullptr;
  //union {
    IWJsonable* jsonable;
    //WLed* asLed;
    //WRelay* asRelay;    
  //};
};

class WThingIO : public WDevice, public IWIterable<WThing> {
 public:
  WThingIO(WNetwork *network)
      : WDevice(network, DEVICE_ID, DEVICE_ID, DEVICE_TYPE_ON_OFF_SWITCH, DEVICE_TYPE_LIGHT) {        
    _items = new WList<WThing>();
    loadFromStore();       
    
    //Configure items
    _items->forEach([this](int index, WThing* thing, const char* id) {
      bool mq = true;// bitRead(thing->config, BIT_CONFIG_PROPERTY_MQTT);
      bool wt = true;// bitRead(thing->config, BIT_CONFIG_PROPERTY_WEBTHING);
      switch (thing->type) {
        case GPIO_TYPE_LED : {        
          WLed* led = (WLed*) thing->jsonable;            
          if (led->linkState()) { this->network()->setStatusLed(led, false); }
          break;
        }                    
      }      
      if (_isOutput(thing->type)) {
        WOutput* output = (WOutput*) thing->jsonable;
        const char* tid = "test";//thing->asLed->id();
        WThing* linkedThing = this->_items->getById(tid);
        if ((tid != nullptr) && (strcmp_P(tid, "") != 0) && (linkedThing == nullptr)) {          
          if ((wt) || (mq)) {
            LOG->debug("add default property '%s'", tid);
            WProperty* prop = WProps::createOnOffProperty(tid);
            prop->visibilityMqtt(mq);
            prop->visibilityWebthing(wt);
            Serial.println("led");
            //Serial.println(thing->asLed->id());
            output->on(prop);
            this->addProperty(prop, tid);
          }
        }
      }
    });
  }

  virtual void loadFromStore() { 
    SETTINGS->removeAllAfter(TG_NUMBER_OF_GPIOS);
    _items->clear();
    _numberOfGPIOs = SETTINGS->setByte(TG_NUMBER_OF_GPIOS, 0, MAX_GPIOS);
    //Read from EEPROM
    for (byte index = 0; index < _numberOfGPIOs->asByte(); index++) {
      _loadThing();
    }
    LOG->debug(F("Loaded %d items from EEPROM. (%d)"), _items->size(), _numberOfGPIOs->asByte());
  }

  virtual void writeToStore() {
    SETTINGS->save();
  }

  void _loadThing(WList<WValue>* list = nullptr) {    
    WThing* thing = new WThing();
    //type
    WValue* gType = SETTINGS->setByte(nullptr, GPIO_TYPE_UNKNOWN);
    thing->type = (list == nullptr ? (WGpioType) gType->asByte() : _gpioTypeOf(list->getById(WC_TYPE)));
    gType->asByte(thing->type);
    //config byte
    WValue* config = SETTINGS->setByte(nullptr, 0b00000011);    
    thing->config = config->asByte();
    //json
    WValue* wt = (list != nullptr ? list->getById(WC_WEBTHING) : nullptr);
    bitWrite(thing->config, BIT_CONFIG_PROPERTY_WEBTHING, (wt != nullptr ? wt->asBool() : true));      
    Serial.println(".c");
    WValue* mq = (list != nullptr ? list->getById(WC_MQTT) : nullptr);
    bitWrite(thing->config, BIT_CONFIG_PROPERTY_MQTT, (mq != nullptr ? mq->asBool() : true));
    Serial.println(".d");
    config->asByte(thing->config);
    if (thing->type != GPIO_TYPE_UNKNOWN) {
      _items->add(thing);

      IWStorable* storable = nullptr;
      IWJsonable* jsonable = nullptr;
      switch (thing->type) {
        case GPIO_TYPE_LED : {
          Serial.println("led gefunden");
          WLed* led = new WLed(NO_PIN);
          thing->jsonable = led;          
          this->addOutput(led);
          storable = led;
          break;
        }  
        default :
          Serial.print("unknown: ");
          Serial.println(thing->type);
      }
      if (storable != nullptr) storable->loadFromStore();  
      if (list != nullptr) thing->jsonable->loadFromJson(list);  
    } else {
      delete thing;
    }
  }

  virtual WFormResponse* loadFromJson(WList<WValue>* list) {          
    LOG->debug("list items count: %d", list->size());
    SETTINGS->removeAllAfter(TG_NUMBER_OF_GPIOS);
    _items->clear();
    _numberOfGPIOs = SETTINGS->setByte(TG_NUMBER_OF_GPIOS, 0, MAX_GPIOS);
    
    list->forEach([this](int index, WValue* value, const char* key) {
      LOG->debug("key '%s' / value '%s'", key, value->toString());
      if (value->type() == LIST) { 
        _loadThing(value->asList());       
      }
    });
    _numberOfGPIOs->asByte(_items->size());
    LOG->debug(F("Loaded %d items from json"), _items->size());
    SETTINGS->save();
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
    WJson* json = new WJson(stream);
    json->beginArray();
    _items->forEach([this, json](int index, WThing* thing, const char* id) {
      json->beginObject();
      json->propertyByte(WC_TYPE, thing->type);
      json->propertyString(WC_TYPE, S_GPIO_TYPE[thing->type], nullptr);
      
      thing->jsonable->toJson(json);
      json->endObject();

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
    json->endArray();
    delete json;    
  }

  WValue* numberOfGPIOs() { return _numberOfGPIOs; }

 protected:
  WValue* _numberOfGPIOs;
  WList<WThing>* _items;


  bool _isOutput(byte type) { return ((type >= GPIO_TYPE_LED) && (type < GPIO_TYPE_BUTTON)); }
  bool _isInput(byte type) { return ((type >= GPIO_TYPE_BUTTON) && (type < GPIO_TYPE_UNKNOWN)); }

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