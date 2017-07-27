//
// Created by cout970 on 2017-07-10.
//

#ifndef DRIVER_DEVICES_H
#define DRIVER_DEVICES_H

#include "types.h"

struct device_header {
    i8 online;    //the device is on or off
    i8 type;      //id of the device, must be unique
    i16 status;
};

#define DEVICE_TYPE_FLOPPY_DRIVE 0
#define DEVICE_TYPE_MONITOR 1
#define DEVICE_TYPE_NETWORK_CARD 2

#endif //DRIVER_DEVICES_H
