#ifndef W_ESP_THING_IO_H
#define W_ESP_THING_IO_H

#include "WDevice.h"
#include "WSwitch.h"
#include "WRelay.h"
#include "WProperty.h"

class WEspThingIO: public WDevice {
public:
	WEspThingIO(WNetwork* network)
	    	: WDevice(network, "switch", "switch", DEVICE_TYPE_ON_OFF_SWITCH) {

  }

protected:

private:

};

#endif
