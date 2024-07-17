#include <Arduino.h>
#include "WNetwork.h"
#include "WThingIO.h"
#include "html/WEspThingPages.h"

WNetwork *network;

void setup() {
  if (DEBUG) {
		Serial.begin(9600);
	}
	APPLICATION = "WEspThingIO";
	VERSION = "1.50";
	FLAG_SETTINGS = 0x46;
	DEBUG = true;
	//Network
	network = new WNetwork(NO_LED);
	//Device
	network->addDevice(new WThingIO(network));	

	//this->addCustomPage(WC_WIFI, [this]() { return new WNetworkPage(); });
	network->addCustomPage("thathing", [](){ 
		Serial.println("initialzer called");
		return new WThingPage(); 
	});
}

void loop() {
  network->loop(millis());	
	delay(50);
}