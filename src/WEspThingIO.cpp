#include <Arduino.h>
#include "WNetwork.h"
#include "WEspThingIO.h"

#define APPLICATION "WEspThingIO"
#define VERSION "1.37"
#define FLAG_SETTINGS 0x12
#define DEBUG false

WNetwork* network;
WDevice* device;

void setup() {
  if (DEBUG) {
		Serial.begin(9600);
	}
	//Network
	network = new WNetwork(DEBUG, APPLICATION, VERSION, NO_LED, FLAG_SETTINGS);
	//Device
	device = new WEspThingIO(network);
	network->addDevice(device);	
}

void loop() {
  network->loop(millis());	
	delay(50);
}