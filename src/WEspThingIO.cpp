#include <Arduino.h>
#include "WNetwork.h"
#include "WThingIO.h"
#include "html/WThingPages.h"

WNetwork *network;

void setup() {
  if (DEBUG) {
		Serial.begin(9600);
	}
	APPLICATION = "WEspThingIO";
	VERSION = "1.50";
	FLAG_SETTINGS = 0x48;
	DEBUG = true;
	//Network
	//network = new WNetwork(NO_LED);
	//Device
	//WThingIO* thing = new WThingIO(network);
	//network->addDevice(thing);	
	//network->addCustomPage("json", [thing](){ return new WJsonPage(thing); }, PSTR("json"));

	LOG->setOutput(&Serial, LOG_LEVEL_NOTICE, true, true);
	delay(2000);
	WJsonParser::asMap("{'LED':{'type':'led'}}");
	LOG->debug("Parsing finished.");
}

void loop() {
  //network->loop(millis());	
	delay(50);
}