//
// Created by cout970 on 2017-07-10.
//

#ifndef DRIVER_DEVICES_H
#define DRIVER_DEVICES_H

#include "base.h"
#include "types.h"

struct device_header {
    Boolean online;    // the device is on or off
    Byte type;         // id of the device, must be unique
    Short status;      // additional information about the state of the device
};

#define DEVICE_TYPE_FLOPPY_DRIVE 0
#define DEVICE_TYPE_MONITOR 1
#define DEVICE_TYPE_NETWORK_CARD 2
#define DEVICE_TYPE_MINING_ROBOT 3

#endif //DRIVER_DEVICES_H
