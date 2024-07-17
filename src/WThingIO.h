#ifndef W_ESP_THING_IO_H
#define W_ESP_THING_IO_H

#include "WDevice.h"

#define DEVICE_ID "switch"

class WThingIO : public WDevice {
 public:
  WThingIO(WNetwork *network)
      : WDevice(network, DEVICE_ID, DEVICE_ID, DEVICE_TYPE_ON_OFF_SWITCH,
                DEVICE_TYPE_LIGHT) {}

 protected:
};

#endif