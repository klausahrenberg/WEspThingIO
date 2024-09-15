#ifndef WThingPages_h
#define WThingPages_h

#include "../WThingIO.h"
#include "html/WPage.h"

/*
[
  {
    "type":"led",
    "gpio":0,
    "inverted":false,
    "linkstate":true
  },
  {
    "type":"group",
    "id":"relay0",
    "title":"Relay",
    "items":[
      {
        "type":"relay",
        "gpio":32,
        "inverted":true
      },
      {
        "type":"relay",
        "gpio":33,
        "inverted":true
      },
    ]
  },
  {
    "type":"mode",
    "id":"on_0",
    "title":"An/Aus",
    "modeId":"mode0",
    "modeTitle":"Auswahl",
    "items":{
      "Relay 25" : {
        "type":"relay",
        "gpio":25,
        "inverted":true
      },
      "Relay 26" : {
        "type":"relay",
        "gpio":26,
        "inverted":true
      }
    }
  }
]
*/

class WJsonPage : public WPage {
 public:
  WJsonPage(WThingIO* gpios) : WPage() {
    _gpios = gpios;
  }

  virtual void createControls(WebControl* parentNode) {        
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

#endif