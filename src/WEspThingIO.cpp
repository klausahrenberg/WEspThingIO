#include <Arduino.h>
#include "WNetwork.h"
#include "WEspThingIO.h"

WNetwork *network;

void setup() {
  if (DEBUG) {
		Serial.begin(9600);
	}
	APPLICATION = "WEspThingIO";
	VERSION = "1.41";
	FLAG_SETTINGS = 0x46;
	DEBUG = true;
	//Network
	network = new WNetwork(NO_LED);
	//Device
	network->addDevice(new WEspThingIO(network));	
}

void loop() {
  network->loop(millis());	
	delay(50);
}