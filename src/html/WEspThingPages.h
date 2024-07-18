#ifndef WEspThingPages_h
#define WEspThingPages_h

#include "../WThingGpios.h"
#include "html/WPage.h"

class WThingPage : public WPage {
 public:
  WThingPage(WThingGpios* gpios) : WPage() {
    _gpios = gpios;
  }

  virtual void createControls(WebControl* parentNode) {    
    /*WebControl* div = new WebControl(WC_DIV, WC_CLASS, WC_WHITE_BOX, nullptr);    
    parentNode->add(div); */
    WebControl* form = new WebForm("thathing", nullptr);
    parentNode->add(form);
    form->add(new WebTextArea("json", "Json Input", "Put your json here"));
    form->add((new WebSubmitButton(PSTR("Save configuration"))));
  }

  virtual WFormResponse* submitForm(WStringList* args) {
    LOG->debug("handle submitform");
    WStringList* agpio = WJsonParser::asMap(args->getById("json"));
    return new WFormResponse(FO_NONE, PSTR("submit ThingPage"));
  }  

 protected:
  WThingGpios* _gpios;

};


class WEspThingPage : public WPage {
 public:
  WEspThingPage() : WPage() {
  }

  virtual void createControls(WebControl* parentNode) {    
    
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

 private:
 

};

#endif