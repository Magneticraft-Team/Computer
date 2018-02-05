//
// Created by cout970 on 2017-07-10.
//
// Api to detect a device, every device has this header, and a unique type, default types are exposed as macros
// Device status info is optional (is there mostly for alignment reasons)
//

#ifndef DRIVER_DEVICES_H
#define DRIVER_DEVICES_H

#include "base.h"
#include "types.h"

// 4 Bytes
struct device_header {
    Boolean online;    // the device is On or Off
    Byte type;         // id of the device, must be unique per type of device
    Short status;      // additional information about the state of the device
};

#define DEVICE_TYPE_FLOPPY_DRIVE 0
#define DEVICE_TYPE_MONITOR 1
#define DEVICE_TYPE_NETWORK_CARD 2
#define DEVICE_TYPE_MINING_ROBOT 3

#endif //DRIVER_DEVICES_H
