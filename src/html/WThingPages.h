#ifndef WThingPages_h
#define WThingPages_h

#include "../WThingIO.h"
#include "html/WebPage.h"

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

//A row of snippet buttons above the json editor. Every button drops a ready
//made block (led, relay, group, ...) into the text area at the caret. While the
//text still parses as an array the block is spliced into it as a real element,
//otherwise it is pushed in as plain text with the commas it needs.
const static char WC_SCRIPT_THING_SNIPPETS_NAME[] PROGMEM = "thingSnippets";
const static char WC_SCRIPT_THING_SNIPPETS[] PROGMEM = R"=====(
const WT_SNIPPETS = {
 "led": {"type":"led","gpio":2,"inverted":false,"linkstate":true},
 "relay": {"type":"relay","gpio":5,"inverted":false},
 "button": {"type":"button","gpio":0,"inverted":true,"longPress":false},
 "switch": {"type":"switch","gpio":4,"inverted":false},
 "pwm": {"type":"pwm","gpio":5},
 "ws2812": {"type":"ws2812","gpio":3,"leds":16},
 "group": {"type":"group","id":"group0","title":"Group","items":[]},
 "mode": {"type":"mode","id":"mode0","title":"Mode","modeId":"mode0","modeTitle":"Selection","items":{}}
};
//Which element of the outer array the caret sits in: the commas at depth 1
//before it, strings skipped so a comma in a value does not count.
function thingSnippetIndex(text, pos) {
 var depth = 0, inStr = false, esc = false, count = 0;
 for (var i = 0; i < pos && i < text.length; i++) {
  var c = text.charAt(i);
  if (inStr) {
   if (esc) esc = false; else if (c == '\\') esc = true; else if (c == '"') inStr = false;
   continue;
  }
  if (c == '"') inStr = true;
  else if (c == '[' || c == '{') depth++;
  else if (c == ']' || c == '}') depth--;
  else if (c == ',' && depth == 1) count++;
 }
 return count;
}
function thingSnippet(id, key) {
 var ta = document.getElementById(id);
 var tpl = WT_SNIPPETS[key];
 if (!ta || !tpl) return;
 ta.focus();
 var start = ta.selectionStart, end = ta.selectionEnd, val = ta.value;
 var block = JSON.stringify(tpl, null, 1);
 if (val.trim() == '') {
  ta.value = '[\n' + block + '\n]';
  ta.selectionStart = ta.selectionEnd = ta.value.length;
  ta.dispatchEvent(new Event('input'));
  return;
 }
 try {
  var arr = JSON.parse(val);
  if (Array.isArray(arr)) {
   arr.splice(thingSnippetIndex(val, start), 0, tpl);
   ta.value = JSON.stringify(arr, null, 1);
   ta.dispatchEvent(new Event('input'));
   return;
  }
 } catch (e) {}
 var before = val.slice(0, start).replace(/\s+$/, '');
 var after = val.slice(end).replace(/^\s+/, '');
 var ins = (before ? (/[\[{,]$/.test(before) ? '\n' : ',\n') : '') + block + (after && !/^\s*[\]},]/.test(after) ? ',' : '');
 ta.value = before + ins + '\n' + after;
 ta.selectionStart = ta.selectionEnd = (before + ins).length;
 ta.dispatchEvent(new Event('input'));
}
)=====";

const static char WC_STYLE_THING_SNIPPETS[] PROGMEM = "display:flex;flex-wrap:wrap;gap:.4rem;margin:.6rem 0";
const static char WC_STYLE_THING_SNIPPETS_ID[] PROGMEM = ".snips";

class WThingSnippetBar : public WebControl {
 public:
  WThingSnippetBar(const char* targetId) : WebControl(WC_DIV, WC_CLASS, PSTR("snips"), nullptr) {
    _addButton(targetId, PSTR("led"), PSTR("+ LED"));
    _addButton(targetId, PSTR("relay"), PSTR("+ Relay"));
    _addButton(targetId, PSTR("button"), PSTR("+ Button"));
    _addButton(targetId, PSTR("switch"), PSTR("+ Switch"));
    _addButton(targetId, PSTR("pwm"), PSTR("+ PWM"));
    _addButton(targetId, PSTR("ws2812"), PSTR("+ WS2812"));
    _addButton(targetId, PSTR("group"), PSTR("+ Group"));
    _addButton(targetId, PSTR("mode"), PSTR("+ Mode"));
  }

  virtual void createStyles(WStringList* styles) {
    styles->add(CSS_BUTTON_STYLE, WC_BUTTON);
    styles->add(CSS_BUTTON_HOVER_STYLE, CSS_BUTTON_HOVER_ID);
    styles->add(WC_STYLE_THING_SNIPPETS, WC_STYLE_THING_SNIPPETS_ID);
    WebControl::createStyles(styles);
  }

  virtual void createScripts(WStringList* scripts) {
    WebControl::createScripts(scripts);
    //the id keeps the script at one copy, however many buttons the bar has
    scripts->add(WC_SCRIPT_THING_SNIPPETS, WC_SCRIPT_THING_SNIPPETS_NAME);
  }

 protected:
  //html reads a button without a type as a submit one, so it would send the
  //form off; type 'button' keeps it to its onclick
  void _addButton(const char* targetId, const char* key, const char* title) {
    WebControl* b = new WebControl(WC_BUTTON, WC_TYPE, WC_BUTTON, nullptr);
    b->content(title);
    char buffer[64];
    snprintf_P(buffer, sizeof(buffer), PSTR("thingSnippet('%s','%S')"), targetId, (const __FlashStringHelper*)key);
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
    form->add(new WThingSnippetBar("json"));
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

#endif
