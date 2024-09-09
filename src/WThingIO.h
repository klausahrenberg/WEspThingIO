#ifndef W_ESP_THING_IO_H
#define W_ESP_THING_IO_H

#include "WDevice.h"
#include "hw/WRelay.h"
#include "hw/W2812.h"

#define DEVICE_ID "switch"
#define MAX_GPIOS 12
#define GPIO_DEFAULT_ID "^g.%d"
#define GPIO_DEFAULT_ID_SUB GPIO_DEFAULT_ID ".%d"
#define GPIO_TYPE_COUNT 9

#define BIT_CONFIG_ID 0
#define BIT_CONFIG_WEBTHING 1
#define BIT_CONFIG_GROUP 2
#define BIT_CONFIG_MODE 3

const static byte NO_PARENT = 0xFF;
const static char TG_GROUP[] PROGMEM = "group";
const static char TG_MODE[] PROGMEM = "mode";
const static char TG_NUMBER_OF_GPIOS[] PROGMEM = "numberOfGPIOs";
const static char TG_WEBTHING[] PROGMEM = "webthing";

struct WThing { 
  WValue* parent;
  WOutput* gpio;
  virtual ~WThing() {
    if (parent) delete parent;
    //check: if released here, it will crash because will be released twice at clearInputOutputs
    //if (gpio) delete gpio;
  }
};

class WThingIO : public WDevice, public IWIterable<WThing> {
 public:
  WThingIO(WNetwork *network)
      : WDevice(network, DEVICE_ID, DEVICE_ID, DEVICE_TYPE_ON_OFF_SWITCH, DEVICE_TYPE_LIGHT) {        
    _items = new WList<WThing>();
    SETTINGS->add(_numberOfGPIOs, TG_NUMBER_OF_GPIOS);
    _loadFromStore();       
    _configureDevice();
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
      if ((thing->parent == nullptr) || (thing->parent->asByte() == NO_PARENT)) {
        json->beginObject();        
      /*if (thing->id) json->propertyString(WC_ID, thing->id->asString(), nullptr);
      if (thing->title) json->propertyString(TG_WEBTHING, thing->title->asString(), nullptr);
      if (thing->group) json->propertyString(TG_GROUP, thing->group->asString(), nullptr);
      if (thing->mode) json->propertyString(TG_MODE, thing->mode->asString(), nullptr);
      */
      
      //json->propertyBoolean(TG_WEBTHING, thing->config->asBit(BIT_CONFIG_WEBTHING));
      //json->propertyBoolean(WC_MQTT, thing->config->asBit(BIT_CONFIG_ID));
      
        thing->gpio->toJson(json);
        json->endObject();
      }

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

  virtual WFormResponse* loadFromJson(WList<WValue>* list) {
    _resetDevice();     
    _numberOfGPIOs->asByte(0);
    for (int i = 0; i < list->size(); i++) {
      WValue* value = list->get(i);
      if (value->type() == LIST) { 
        _numberOfGPIOs->asByte(_numberOfGPIOs->asByte() + _loadThing(value->asList(), NO_PARENT));
      }
    }
    SETTINGS->save();
    return new WFormResponse(FO_RESTART);
  }

 protected:
  WValue* _numberOfGPIOs = new WValue((byte) 0);
  WList<WThing>* _items;

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

  virtual void _loadFromStore() { 
    _resetDevice();
    //Read from EEPROM
    int index = 0;
    while (index < _numberOfGPIOs->asByte()) {
      index = index + _loadThing(nullptr, NO_PARENT);
    }
    LOG->debug(F("Loaded %d items from EEPROM. (%d, first level: %d)"), _items->size(), _numberOfGPIOs->asByte(), index);
  }

  WOutput* _loadGpio(WGpioType type) {
    switch (type) {
      case GPIO_TYPE_LED : return new WLed();
      case GPIO_TYPE_RELAY: return new WRelay(NO_PIN);
      case GPIO_TYPE_RGB_LED: return new W2812Led(NO_PIN, 22);//gConfig->byteArrayValue(BYTE_NO_OF_LEDS));
      case GPIO_TYPE_MODE: return new WMode();
      case GPIO_TYPE_GROUP: return new WGroup();
      default :
        LOG->debug(F("create group"));
        return nullptr;
      }
  }

  bool _loadThing(WList<WValue>* json, byte parent) {    
    byte result = false;
    WThing* thing = new WThing();
    /*//parent
    WValue* gParent = new WValue(parent);
    SETTINGS->add(gParent, nullptr);
    if (gParent->asByte() == parent) {*/
      //type
      WValue* gType = new WValue((byte) GPIO_TYPE_UNKNOWN);
      SETTINGS->add(gType, nullptr);      
      WGpioType type = (json == nullptr ? (WGpioType) gType->asByte() : _gpioTypeOf(json->getById(WC_TYPE)));
      if (json != nullptr) {
        
        json->forEach([](int index, WValue* item, const char* id) {
          Serial.print(">  ");
          Serial.print(id);
          Serial.print(" / ");
          Serial.println(item->asString());
        });
        //Serial.println(json->getById(WC_TYPE)->asString());
      }
      gType->asByte(type);
      LOG->debug(F("type is '%s'"), S_GPIO_TYPE[type]);
      WValue* childCount = nullptr;
      if ((type == GPIO_TYPE_GROUP) || (type == GPIO_TYPE_MODE)) {
        childCount = new WValue(BYTE);
        SETTINGS->add(childCount, nullptr);
        /*if (json != nullptr) {
          Serial.println("has items?");
          Serial.println(json->existsId(WC_ITEMS));
          json->ifExistsId(WC_ITEMS, [childCount] (WValue* item) { 
            Serial.println("items exists");
            if (item->type() == LIST) {
              childCount->asByte(item->asList()->size());
            } 
          });
          Serial.println("has items!");
        }*/
        //LOG->debug(F("child count: %d"), childCount->asByte());
      }
    
      Serial.println(".d ");
      if (type != GPIO_TYPE_UNKNOWN) {
        thing->gpio = _loadGpio(type);        
        if (thing->gpio != nullptr) {
          thing->gpio->registerSettings();
          if (json != nullptr) thing->gpio->fromJson(json);
          _items->add(thing);
          Serial.println("add....");
          Serial.println(parent != NO_PARENT);
          Serial.println(thing->gpio->isOutput());
          Serial.println(type);
          if ((parent != NO_PARENT) && (thing->gpio->isOutput())) {
            WThing* p = _items->get(parent);
            if (p->gpio->type() == GPIO_TYPE_GROUP) {
              Serial.println(" is group");
              ((WGroup*) p->gpio)->addItem((WOutput*) thing->gpio, nullptr);
            } else if (p->gpio->type() == GPIO_TYPE_MODE) {
              Serial.println("is mode");
              ((WMode*) p->gpio)->addItem((WOutput*) thing->gpio, "tbi");
            } else {
              Serial.println(" is mist");
            }
          }
          result = true;
          if (childCount != nullptr) {
            //look now for childrens
            byte pIndex = _items->size() - 1;
            if (json != nullptr) {
              //json read, 
              childCount->asByte(0);
              if (json->existsId(WC_ITEMS)) {
                //tbi
                WValue* list = json->getById(WC_ITEMS);
                if (list->type() == LIST) {                  
                  list->asList()->forEach([this, childCount, pIndex] (int index, WValue* subThing, const char* id) {
                    if (subThing->type() == LIST) {
                      if (_loadThing(subThing->asList(), pIndex)) {
                        childCount->asByte(childCount->asByte() + 1);
                      }
                    }
                  });
                }                
              }
              LOG->debug(F("Json: Childs for parent index %d, has %d subitems"), pIndex, childCount->asByte());                 
            } else {
              //EEPROM read
              LOG->debug(F("EEPRom: Childs for parent index %d, has %d subitems"), pIndex, childCount->asByte());                  
              for (int i = 0; i < childCount->asByte(); i++) {
                _loadThing(json, pIndex);
              }  
            }
            LOG->debug(F("Loaded %d childs"), childCount->asByte());
          }
        } else {
          delete thing;
        }  
      } else {
        delete thing;
      }
    /*} else {
      LOG->debug(F("Can't read EEPROM, storage tree corrupted"));
      result = _numberOfGPIOs->asByte();
    }*/
    return result;
  }

  void _resetDevice() {
    SETTINGS->removeAllAfter(TG_NUMBER_OF_GPIOS);
    network()->setStatusLed(nullptr);
    _items->clear();
    this->clearInAndOutputs();    
  }

  void _configureDevice() {
    Serial.println("_configureDevice a");
    //Configure items
    //1. Modes
    _items->forEach([this](int index, WThing* thing, const char* id) {
      if (thing->gpio->type() == GPIO_TYPE_MODE) {

      }
    });  
    Serial.println("_configureDevice b");
    _items->forEach([this](int index, WThing* thing, const char* id) {
      //Special handling, link led, config button, etc.
      switch (thing->gpio->type()) {
        case GPIO_TYPE_LED : {        
          Serial.println("_configureDevice c");
          WLed* led = (WLed*) thing->gpio;            
          if (led->linkState()) { 
            Serial.println("yepp linkstate");
            this->network()->setStatusLed(led); 
          }
          Serial.println("_configureDevice d");
          break;
        }                    
      }
      //check outputs for creating properties etc.
      if (thing->gpio->isOutput()) {
        Serial.println("_configureDevice e");
        WOutput* output = (WOutput*) thing->gpio;
        Serial.println("_configureDevice f");
        
        /*WThing* linkedThing = this->_items->getById(thing->id != nullptr ? thing->id->asString() : nullptr);
        if ((thing->id != nullptr) && (!thing->id->isStringEmpty()) && (linkedThing == nullptr)) {      
          if ((wt) || (id)) {
            LOG->debug("add default property '%s'", thing->id->asString());
            WProperty* prop = WProps::createOnOffProperty(thing->id->asString());
            prop->visibilityMqtt(id);
            prop->visibilityWebthing(wt);
            Serial.println("led");
            //Serial.println(thing->asLed->id());
            output->on(prop);
            this->addProperty(prop, thing->id->asString());
          }
        }*/
        Serial.println("_configureDevice h");
      }
    });
    //2. Mode
    _items->forEach([this](int index, WThing* thing, const char* id) {
      if (thing->gpio->isGroupOrMode()) {
        Serial.println("create group property");
        WGroup* group = (WGroup*) thing->gpio;        
        WProperty* onOffProp = WProps::createOnOffProperty(group->title()->asString());
        onOffProp->visibilityMqtt(!group->id()->isStringEmpty());
        onOffProp->visibilityWebthing(!group->title()->isStringEmpty());
        //onOffProp->addListener([this, onOffProp]() { _notifyGroupedChange(onOffProp, FIRST_NAME); });
        group->on(onOffProp);
        
        this->addProperty(onOffProp, group->id()->asString());        

        if (thing->gpio->type() == GPIO_TYPE_MODE) {
          WMode* mode = (WMode*) thing->gpio;
          WProperty* modeProp = WProps::createStringProperty(mode->modeTitle()->asString());
          //tbi SETTINGS->add(modeProp->value(), mName);
          modeProp->visibilityMqtt(!mode->modeId()->isStringEmpty());
          modeProp->visibilityWebthing(!mode->modeTitle()->isStringEmpty());
          //modeProp->addListener([this, modeProp]() { _notifyGroupedChange(modeProp, SECOND_NAME); });
          this->addProperty(modeProp, mode->modeId()->asString());
        }
      }
    });  
    Serial.println("_configureDevice i");
    
  }
  
};

#endif