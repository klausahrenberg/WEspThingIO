#ifndef W_ESP_THING_IO_H
#define W_ESP_THING_IO_H

#include "WDevice.h"
#include "hw/W2812.h"
#include "hw/WHtu21D.h"
#include "hw/WRelay.h"
#include "hw/WSht30.h"
#include "hw/WSwitch.h"
#include "hw/WPwmDimmer.h"

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
  byte parent;
  WGpio* gpio;
  virtual ~WThing() {
    // check: if released here, it will crash because will be released twice at clearInputOutputs
    // if (gpio) delete gpio;
  }
};

class WThingIO : public WDevice, public IWIterable<WThing> {
 public:
  WThingIO(WNetwork* network)
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
      if (thing->parent == NO_PARENT) {
        json->beginObject();
        thing->gpio->toJson(json);
        json->endObject();
      }
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
        _numberOfGPIOs->asByte(_numberOfGPIOs->asByte() + _loadThing(value->asList(), nullptr, NO_PARENT));
      }
    }
    SETTINGS->save();
    return new WFormResponse(FO_RESTART);
  }

 protected:
  WValue* _numberOfGPIOs = new WValue((byte)0);
  WList<WThing>* _items;

  WGpioType _gpioTypeOf(WValue* jType) {
    if (jType != nullptr) {
      for (byte b = 0; b < GPIO_TYPE_COUNT; b++) {
        if (jType->equalsString(S_GPIO_TYPE[b])) {
          return (WGpioType)b;
        }
      }
    }
    return GPIO_TYPE_UNKNOWN;
  }

  virtual void _loadFromStore() {
    _resetDevice();
    // Read from EEPROM
    int index = 0;
    while (index < _numberOfGPIOs->asByte()) {
      index = index + _loadThing(nullptr, nullptr, NO_PARENT);
    }
    LOG->debug(F("Loaded %d items from EEPROM. (%d, first level: %d)"), _items->size(), _numberOfGPIOs->asByte(), index);
  }

  WGpio* _loadGpio(WGpioType type) {
    switch (type) {
      case GPIO_TYPE_BUTTON:
      case GPIO_TYPE_SWITCH:
        return new WSwitch(type);
      case GPIO_TYPE_HTU21:
        return new WHtu21D();
      case GPIO_TYPE_SHT30:
        return new WSht30();
      case GPIO_TYPE_LED:
        return new WLed();
      case GPIO_TYPE_RELAY:
        return new WRelay(NO_PIN);
      case GPIO_TYPE_RGB_WS2812:
      case GPIO_TYPE_RGB_PL9823:
        return new W2812Led(type);  // gConfig->byteArrayValue(BYTE_NO_OF_LEDS));
      case GPIO_TYPE_PWM:
        return new WPwmDimmer();  
      case GPIO_TYPE_MODE:
        return new WMode();
      case GPIO_TYPE_GROUP:
        return new WGroup();
      default:
        LOG->debug(F("create group"));
        return nullptr;
    }
  }

  bool _loadThing(WList<WValue>* json, WValue* jsonId, byte parent) {
    byte result = false;
    WThing* thing = new WThing();
    thing->parent = parent;
    // type
    WValue* gType = new WValue((byte)GPIO_TYPE_UNKNOWN);
    SETTINGS->add(gType, nullptr);
    WGpioType type = (json == nullptr ? (WGpioType)gType->asByte() : (json->existsId(WC_TYPE) ? _gpioTypeOf(json->getById(WC_TYPE)) : GPIO_TYPE_GROUP));
    gType->asByte(type);
    LOG->debug(F("GPIO type is '%s'"), S_GPIO_TYPE[type]);
    WValue* childCount = nullptr;
    if ((type == GPIO_TYPE_GROUP) || (type == GPIO_TYPE_MODE)) {
      childCount = new WValue(BYTE);
      SETTINGS->add(childCount, nullptr);
    }
    if (type != GPIO_TYPE_UNKNOWN) {
      thing->gpio = _loadGpio(type);
      if (thing->gpio != nullptr) {
        thing->gpio->registerSettings();
        if (json != nullptr) thing->gpio->fromJson(json);
        _items->add(thing);
        if ((parent != NO_PARENT) && ((thing->gpio->isOutput()) || (thing->gpio->isInput()))) {
          WThing* p = _items->get(parent);
          if (p->gpio->type() == GPIO_TYPE_GROUP) {
            ((WGroup*)p->gpio)->addItem((WGpio*)thing->gpio, nullptr);
          } else if (p->gpio->type() == GPIO_TYPE_MODE) {
            ((WMode*)p->gpio)->addItem((WGpio*)thing->gpio, (jsonId != nullptr ? jsonId->asString() : nullptr));
          }
        }
        result = true;
        if (childCount != nullptr) {
          // look now for childrens
          byte pIndex = _items->size() - 1;
          if (json != nullptr) {
            // json read,
            childCount->asByte(0);
            if (json->existsId(WC_ITEMS)) {
              // tbi
              WValue* list = json->getById(WC_ITEMS);
              if (list->type() == LIST) {
                list->asList()->forEach([this, gType, childCount, pIndex](int index, WValue* subThing, const char* id) {
                  if (subThing->type() == LIST) {
                    WValue* childId = nullptr;
                    if (gType->asByte() == GPIO_TYPE_MODE) {
                      // id
                      childId = new WValue(STRING);
                      SETTINGS->add(childId, nullptr);
                      childId->asString(id);
                    }
                    if (_loadThing(subThing->asList(), childId, pIndex)) {
                      childCount->asByte(childCount->asByte() + 1);
                      if (childId != nullptr) childId->asString(id);
                    }
                  }
                });
              }
            }
            LOG->debug(F("Json: Childs for parent index %d, has %d subitems"), pIndex, childCount->asByte());
          } else {
            // EEPROM read
            LOG->debug(F("EEPRom: Childs for parent index %d, has %d subitems"), pIndex, childCount->asByte());
            for (int i = 0; i < childCount->asByte(); i++) {
              WValue* childId = nullptr;
              if (gType->asByte() == GPIO_TYPE_MODE) {
                // id
                childId = new WValue(STRING);
                SETTINGS->add(childId, nullptr);
              }
              _loadThing(json, childId, pIndex);
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
    return result;
  }

  void _resetDevice() {
    SETTINGS->removeAllAfter(TG_NUMBER_OF_GPIOS);
    network()->setStatusLed(nullptr);
    _items->clear();
    this->clearGpios();
  }

  void _configureDevice() {
    // Configure items   
    // 1. Groups and Modes
    _items->forEach([this](int index, WThing* thing, const char* id) {
      if (thing->gpio->isGroupOrMode()) {
        WGroup* group = (WGroup*)thing->gpio;
        LOG->debug(F("Create on property '%s'"), group->id()->asString());
        WProperty* onOffProp = WProps::createOnOffProperty(group->title()->asString());
        onOffProp->visibilityMqtt(!group->id()->isStringEmpty());
        onOffProp->visibilityWebthing(!group->title()->isStringEmpty());
        // onOffProp->addListener([this, onOffProp]() { _notifyGroupedChange(onOffProp, FIRST_NAME); });
        group->property(onOffProp);

        this->addProperty(onOffProp, group->id()->asString());

        if (thing->gpio->type() == GPIO_TYPE_MODE) {
          WMode* mode = (WMode*)thing->gpio;
          LOG->debug(F("Create mode property '%s'"), mode->modeId()->asString());
          WProperty* modeProp = WProps::createStringProperty(mode->modeTitle()->asString());
          // tbi SETTINGS->add(modeProp->value(), mName);
          modeProp->visibilityMqtt(!mode->modeId()->isStringEmpty());
          modeProp->visibilityWebthing(!mode->modeTitle()->isStringEmpty());
          mode->modeProp(modeProp);
          // modeProp->addListener([this, modeProp]() { _notifyGroupedChange(modeProp, SECOND_NAME); });
          this->addProperty(modeProp, mode->modeId()->asString());
        }
      }
    });
    // 2. Special handling for different GPIOs
    _items->forEach([this](int index, WThing* thing, const char* id) {
      // Special handling, link led, config button, etc.
      switch (thing->gpio->type()) {
        case GPIO_TYPE_LED: {
          WLed* led = (WLed*)thing->gpio;
          if (led->linkState()) {
            this->network()->setStatusLed(led);
          }
          break;
        }
        case GPIO_TYPE_RGB_WS2812:
        case GPIO_TYPE_RGB_PL9823: {
          LOG->debug(F("Create LED program property"));
          W2812Led* rgb = (W2812Led*)thing->gpio;
          WProperty* modeProp = WProps::createStringProperty("RGB Mode");
          //SETTINGS->add(modeProp->value(), nullptr);
          this->addProperty(modeProp, "rgb_mode");          
          // Enums
          for (byte i = 0; i < rgb->countModes(); i++) {
            modeProp->addEnumString(rgb->modeTitle(i));
          }
          // Default/Initial value
          if (!modeProp->isStringEmpty()) {
            rgb->setRgbModeByTitle(modeProp->asString());
          } else {
            modeProp->asString(rgb->modeTitle(rgb->rgbMode()));
          }                    
          // Change notification
          modeProp->addListener([this, modeProp, rgb]() {
            rgb->setRgbModeByTitle(modeProp->asString());
            //SETTINGS->save();
          });
          break;
        }
        case GPIO_TYPE_PWM: {
          LOG->debug(F("Create brightness property"));
          WPwmDimmer* pwm = (WPwmDimmer*)thing->gpio;
          WProperty* brightness = WProps::createLevelIntProperty("Brightness", 0, 100);
		      brightness->asInt(pwm->level());
		      brightness->unit(UNIT_PERCENT);
		      //network->getSettings()->add(this->brightness);
		      //float v = this->brightness->getInteger();  
          this->addProperty(brightness, "brightness");
		      brightness->addListener([this, brightness, pwm]() {
			      pwm->level(brightness->asInt());
		      });
          break;
        }
      }
      // check outputs for creating properties etc.
      if (thing->gpio->isOutput()) {
        WGpio* output = (WGpio*)thing->gpio;

        /*WThing* linkedThing = this->_items->getById(thing->id != nullptr ? thing->id->asString() : nullptr);
        if ((thing->id != nullptr) && (!thing->id->isStringEmpty()) && (linkedThing == nullptr)) {
          if ((wt) || (id)) {
            LOG->debug("add default property '%s'", thing->id->asString());
            WProperty* prop = WProps::createOnOffProperty(thing->id->asString());
            prop->visibilityMqtt(id);
            prop->visibilityWebthing(wt);
            output->on(prop);
            this->addProperty(prop, thing->id->asString());
          }
        }*/
      }
    });
  }
};

#endif