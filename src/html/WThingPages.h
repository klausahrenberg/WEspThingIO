#ifndef WThingPages_h
#define WThingPages_h

#include "../WThingIO.h"
#include "html/WPage.h"


/*
[
  {
    "type":"group",
    "title":"Schalter",
    "items":[
      {
        "type":"led",
        "gpio":22,
      },
      {
        "type":"relay",
        "gpio":24,
      }
    ]
  },
  {
    "type":"led",
    "gpio":2,
    "linkstate":true,
  }
]


[
  {
    "type":"group",
    "title":"Schalter",
    "items":{
      "LED":{
        "type":"led",
        "gpio":22,
      },
      "Relay":{
        "type":"relay",
        "gpio":24,
      }
    }
  },
  {
    "type":"led",
    "gpio":2,
    "linkstate":true,
  }
]

[
  {
    "type":"led",
    "gpio":2,
    "linkstate":true,
  }
]

[
  {
    "webthing":false,
    "mqtt":false,
    "type":"led",
    "gpio":2,
    "linkstate":true,
    "id":"led0",
    "webthing":true
  }
]

*/

const static char W_JSON_EXAMPLE[] PROGMEM = R"=====(
[
 {
  'type':'button',
  'gpio':4,
  'switchDirect':'led0',
 }
]
)====="; 

class WJsonPage : public WPage {
 public:
  WJsonPage(WThingIO* gpios) : WPage() {
    _gpios = gpios;
  }



  virtual void createControls(WebControl* parentNode) {    
    /*WebControl* div = new WebControl(WC_DIV, WC_CLASS, WC_WHITE_BOX, nullptr);    
    parentNode->add(div); */
    WebControl* form = new WebForm("json", nullptr);
    parentNode->add(form);
    form->add(new WebLabel(_gpios->numberOfGPIOs()->toString()));
    form->add(new WebTextArea("json", "Json Input", [this](Print* stream){
      _gpios->toJson(stream);
    }));
    form->add((new WebSubmitButton(PSTR("Save configuration"))));
  }

  virtual WFormResponse* submitForm(WList<WValue>* args) {  
    LOG->debug("handle submitform: %s", args->getById("json")->toString());
    _gpios->loadFromJson(WJsonParser::asMap(args->getById("json")->asString()));
    return new WFormResponse(FO_RESTART);
  }  

 protected:
  WThingIO* _gpios;

};


class WThingPage : public WPage {
 public:
  WThingPage(WThingIO* gpios) : WPage() {
    _gpios = gpios;
  }

  virtual void createControls(WebControl* parentNode) {    
    WebControl* div = new WebControl(WC_DIV, WC_CLASS, WC_WHITE_BOX, nullptr);
    parentNode->add(div);
    //div->add((new WebTable(_gpios))->onPrintRow([this](Print* stream, int index, WThing* thing, const char* id){
      /*
      //WebTable<WValue>::headerCell(stream, id);
      byte aProps = gConfig->byteArrayValue(BYTE_CONFIG);
      bool gr = bitRead(aProps, BIT_CONFIG_PROPERTY_GROUPED);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      WebTable<WValue>::dataCell(stream, WValue::ofInt(index).toString());
      //WebTable<WValue>::dataCell(stream, _gpios->getGpioDisplayName(gType));
      stream->print(_gpios->getGpioDisplayName(gType));
      WHtml::command(stream, WC_TABLE_DATA, true, nullptr); 
      if (gr) {
        stream->print("> ");
      }
      const char* gName = (gr && (gType == GPIO_TYPE_MERGE) ? _gpios->getSubString(index, SECOND_NAME) : _gpios->getSubString(index, FIRST_NAME));
      if (gName != nullptr) {
        stream->print(gName);
      }
      WHtml::command(stream, WC_TABLE_DATA, false, nullptr);       */
    //}));
  }

  /*virtual void _printConfigPage(WPage* page) {
    if (_numberOfGPIOs->asByte() < MAX_GPIOS) {
      page->print(F("<table  class='tt'>"));
      WHtml::tr(page);
      page->print(F("<td colspan='4'>"));
      page->printf(HTTP_BUTTON, HTTP_USE_TEMPLATE, VALUE_GET, "Use a template...");
      WHtml::tdEnd(page);
      WHtml::trEnd(page);
      WHtml::tr(page);
      WHtml::td(page);
      page->print(F("Add output:"));
      WHtml::tdEnd(page);
      // LED
      WHtml::td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_LED, VALUE_GET, "LED");
      WHtml::tdEnd(page);
      // Relay
      WHtml::td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_RELAY, VALUE_GET, "Relay");
      WHtml::tdEnd(page);
      // Dimmer
      WHtml::td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_DIMMER, VALUE_GET, "Dimmer");
      WHtml::tdEnd(page);
      // RGB LED
      WHtml::td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_RGB_LED, VALUE_GET, "RGB LED");
      WHtml::tdEnd(page);
      // Switch
      WHtml::trEnd(page);
      WHtml::tr(page);
      WHtml::td(page);
      page->print(F("Add input:"));
      WHtml::tdEnd(page);
      WHtml::td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_SWITCH, VALUE_GET, "Switch");
      WHtml::tdEnd(page);
      // Button
      WHtml::td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_BUTTON, VALUE_GET, "Button");
      WHtml::tdEnd(page);
      // Temp sensor
      WHtml::td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_TEMP_SENSOR, VALUE_GET, "Temp sensor");
      WHtml::tdEnd(page);
      WHtml::trEnd(page);
      WHtml::tr(page);
      WHtml::td(page);
      page->print(F("Add grouped:"));
      WHtml::tdEnd(page);
      WHtml::td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_MODE, VALUE_GET, "On/Mode");
      WHtml::tdEnd(page);
      WHtml::td(page);
      page->printf(HTTP_BUTTON, HTTP_ADD_MERGE, VALUE_GET, "Merge");
      WHtml::tdEnd(page);
      WHtml::trEnd(page);
      page->print(F("</table>"));
    }
    // Table with already stored MAX_GPIOS
    page->printf(HTTP_DIV_ID_BEGIN, "gd");
    WHtml::table(page, "st");
    WHtml::tr(page);
    WHtml::th(page); 
    WHtml::thEnd(page);
    WHtml::th(page);
    page->print("Type");
    WHtml::thEnd(page);
    WHtml::th(page);
    page->print("GPIO");
    WHtml::thEnd(page);
    WHtml::th(page);
    page->print("Name");
    WHtml::thEnd(page);
    WHtml::th(page); 
    WHtml::thEnd(page);
    WHtml::th(page); 
    WHtml::thEnd(page);
    WHtml::trEnd(page);
    char* pName = new char[8];

    char* pNumber = new char[2];
    char* gNumber = new char[2];
    for (byte i = 0; i < _numberOfGPIOs->asByte(); i++) {
      WProperty* gConfig = _getGpioConfig(i);
      byte aProps = gConfig->byteArrayValue(BYTE_CONFIG);
      bool gr = bitRead(aProps, BIT_CONFIG_PROPERTY_GROUPED);
      byte gType = gConfig->byteArrayValue(BYTE_TYPE);
      sprintf(pName, "gpio_%d", i);
      sprintf(pNumber, "%d", i);
      WHtml::tr(page);
      WHtml::td(page);
      page->print(pNumber);
      WHtml::tdEnd(page);
      WHtml::td(page);
      page->print(_getGpioDisplayName(gType));
      char* targetName = _getGpioTarget(gType);
      WHtml::tdEnd(page);
      WHtml::td(page);
      if (!_isGpioGrouped(gType)) {
        sprintf(gNumber, "%d", gConfig->byteArrayValue(BYTE_GPIO));
        page->print(gNumber);
      }
      WHtml::tdEnd(page);
      WHtml::td(page);
      if (gr) {
        page->print("> ");
      }
      char* gName = (gr && (gType == GPIO_TYPE_MERGE) ? _getSubString(gConfig, SECOND_NAME) : _getSubString(gConfig, FIRST_NAME));
      if (gName != nullptr) {
        page->print(gName);
      }
      WHtml::tdEnd(page);
      WHtml::td(page);
      page->printf(HTTP_BUTTON_VALUE, targetName, "", pNumber, CAPTION_EDIT);
      WHtml::tdEnd(page);
      WHtml::td(page);
      page->printf(HTTP_BUTTON_VALUE, HTTP_REMOVE_GPIO, "cbtn", pNumber,
                   CAPTION_REMOVE);
      WHtml::tdEnd(page);
      WHtml::trEnd(page);
    }
    WHtml::tableEnd(page);
    page->printf(HTTP_DIV_END);

    page->printf(HTTP_CONFIG_PAGE_BEGIN, id());
    page->print(FPSTR(HTTP_CONFIG_SAVE_BUTTON));
  }*/

 protected:
  WThingIO* _gpios;

 private:

};

#endif