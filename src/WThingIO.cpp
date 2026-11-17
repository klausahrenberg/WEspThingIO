#include <Arduino.h>
#include "WNetwork.h"
#include "WThingIO.h"
#include "html/WThingPages.h"

WNetwork* network;

void setup() {
  APPLICATION = "ThingIO";
  VERSION = "1.50";
  FLAG_SETTINGS = 0x65;
  DEBUG = true;
  if (DEBUG) {
    Serial.begin(115200);
  }
  // Network
  network = new WNetwork(NO_LED);
  // Device
  WThingIO* thing = new WThingIO(network);
  network->addDevice(thing);
  network->addWebPage("json", [thing]() { return new WJsonPage(thing); }, PSTR("json"));
}

void loop() {
  network->loop(millis());
  delay(50);
}