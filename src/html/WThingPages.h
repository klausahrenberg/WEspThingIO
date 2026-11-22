#ifndef WThingPages_h
#define WThingPages_h

#include "../WThingIO.h"
#include "html/WebPage.h"
#include "WThingGpioEditor.h"

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

//Two buttons above the json editor: the json goes into a file and comes back
//from one. The file is read and written by the browser, the device only sees
//the text area as it always did.
const static char WC_SCRIPT_THING_FILES_NAME[] PROGMEM = "thingFiles";
const static char WC_SCRIPT_THING_FILES[] PROGMEM = R"=====(
function thingLoadFile(id) {
 var ta = document.getElementById(id), f = document.createElement('input');
 f.type = 'file';
 f.accept = '.json,application/json';
 f.onchange = function() {
  if (!f.files.length) return;
  var r = new FileReader();
  r.onload = function() {
   try { JSON.parse(r.result); } catch (e) { alert('This is no valid json: ' + e.message); return; }
   ta.value = r.result;
   try { ta.value = JSON.stringify(JSON.parse(r.result), null, 1); } catch (e) {}
   ta.dispatchEvent(new Event('input'));
  };
  r.readAsText(f.files[0]);
 };
 f.click();
}
function thingSaveFile(id) {
 var a = document.createElement('a');
 a.href = URL.createObjectURL(new Blob([document.getElementById(id).value], {type: 'application/json'}));
 a.download = location.hostname + '.json';
 a.click();
 URL.revokeObjectURL(a.href);
}
)=====";

const static char WC_STYLE_THING_FILES[] PROGMEM = "display:flex;flex-wrap:wrap;gap:.4rem;margin:.6rem 0";
const static char WC_STYLE_THING_FILES_ID[] PROGMEM = ".files";

class WThingFileBar : public WebControl {
 public:
  WThingFileBar(const char* targetId) : WebControl(WC_DIV, WC_CLASS, PSTR("files"), nullptr) {
    _addButton(targetId, PSTR("Load json file"), PSTR("thingLoadFile"));
    _addButton(targetId, PSTR("Save json file"), PSTR("thingSaveFile"));
  }

  virtual void createStyles(WStringList* styles) {
    styles->add(CSS_BUTTON_STYLE, WC_BUTTON);
    styles->add(CSS_BUTTON_HOVER_STYLE, CSS_BUTTON_HOVER_ID);
    styles->add(WC_STYLE_THING_FILES, WC_STYLE_THING_FILES_ID);
    WebControl::createStyles(styles);
  }

  virtual void createScripts(WStringList* scripts) {
    WebControl::createScripts(scripts);
    scripts->add(WC_SCRIPT_THING_FILES, WC_SCRIPT_THING_FILES_NAME);
  }

 protected:
  //html reads a button without a type as a submit one, so it would send the
  //form off; type 'button' keeps it to its onclick
  void _addButton(const char* targetId, const char* title, const char* function) {
    WebControl* b = new WebControl(WC_BUTTON, WC_TYPE, WC_BUTTON, nullptr);
    b->content(title);
    char buffer[64];
    snprintf_P(buffer, sizeof(buffer), PSTR("%S('%s')"), (const __FlashStringHelper*)function, targetId);
    b->param(WC_ON_CLICK, buffer);
    this->add(b);
  }
};

class WJsonPage : public WebPage {
 public:
  WJsonPage(WThingIO* gpios) : WebPage() {
    _gpios = gpios;
  }

  virtual WebControl* createControls() {
    WebControl* form = new WebForm("json", nullptr);
    form->add(new WebLabel(_gpios->numberOfGPIOs()->toString()));
    form->add(new WThingFileBar("json"));
    form->add(new WebTextArea("json", "Json Input", [this](Print* stream){
      _gpios->toJson(stream);
    }, 20, 80, true));
    form->add((new WebSubmitButton(PSTR("Save configuration"))));
    return form;
  }

  virtual WFormResponse submitForm(WList<WValue>* args) {
    LOG->debug("handle submitform: %s", args->getById("json")->toString());
    _gpios->loadFromJson(WJsonParser::asMap(args->getById("json")->asString()));
    return WFormResponse(FO_RESTART);
  }

 protected:
  WThingIO* _gpios;

};

/**
 * The gpio page: the same configuration as the json page, but as a form. The
 * text area holds the json and is hidden by the editor script, which builds the
 * form from it and writes every change back. The script is sent from the flash
 * by the thing, see WThingIO::bindWebServerCalls.
 */
class WGpioPage : public WebPage {
 public:
  WGpioPage(WThingIO* gpios) : WebPage() {
    _gpios = gpios;
  }

  virtual WebControl* createControls() {
    WebControl* form = new WebForm("gpio", nullptr);
#ifdef ARDUINO_ARCH_ESP8266
    form->add(new WebControl(WC_DIV, WC_ID, "ge", PSTR("data-chip"), "8266", nullptr));
#else
    form->add(new WebControl(WC_DIV, WC_ID, "ge", PSTR("data-chip"), "32", nullptr));
#endif
    form->add(new WebTextArea("json", "Json", [this](Print* stream) { _gpios->toJson(stream); }, 8, 80, true));
    form->add(new WebSubmitButton(PSTR("Save configuration")));
    form->add(new WebControl(WC_SCRIPT, PSTR("src"), "/gpio.js", nullptr));
    return form;
  }

  virtual WFormResponse submitForm(WList<WValue>* args) {
    WValue* json = (args != nullptr ? args->getById("json") : nullptr);
    WList<WValue>* list = (json != nullptr ? WJsonParser::asMap(json->asString()) : nullptr);
    if (list == nullptr) return WFormResponse(FO_NONE, PSTR("The configuration could not be read."));
    _gpios->loadFromJson(list);
    return WFormResponse(FO_RESTART);
  }

 protected:
  WThingIO* _gpios;
};

#endif
