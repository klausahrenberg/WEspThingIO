#include <Arduino.h>
#include "WNetwork.h"
#include "WThingIO.h"
#include "html/WThingPages.h"

WNetwork* network;

void setup() {
  APPLICATION = "ThingIO";
  VERSION = "1.73";
  FLAG_SETTINGS = 0x67;
  setDebug(false);
  if (DEBUG) {
    Serial.begin(115200);
  }
  // Network
  network = new WNetwork(NO_LED);
  // Device
  WThingIO* thing = new WThingIO(network);
  network->addDevice(thing);
  network->addWebPage("gpio", [thing]() { return new WGpioPage(thing); }, PSTR("Configure GPIO"));
  network->addWebPage("json", [thing]() { return new WJsonPage(thing); }, PSTR("json"));
}

void loop() {
  network->loop(millis());
  delay(50);
}