#include <Arduino.h>
#include "WNetwork.h"
#include "WThingIO.h"
#include "html/WThingPages.h"

WNetwork *network;

void setup() {
  if (DEBUG) {
		Serial.begin(9600);
	}
	APPLICATION = "ThingIO";
	VERSION = "1.50";
	FLAG_SETTINGS = 0x63;
	DEBUG = true;
	//Network
	network = new WNetwork(NO_LED);
	//Device
	WThingIO* thing = new WThingIO(network);
	network->addDevice(thing);	
	network->addCustomPage("json", [thing](){ return new WJsonPage(thing); }, PSTR("json"));	
}

void loop() {
  network->loop(millis());	
	delay(50);
}