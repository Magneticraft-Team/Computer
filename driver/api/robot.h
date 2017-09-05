//
// Created by cout970 on 2017-08-23.
//

#ifndef MAGNETICRAFTCOMPUTER_ROBOT_H
#define MAGNETICRAFTCOMPUTER_ROBOT_H

#include "devices.h"

struct mining_robot {
/*   0 0x00 */    struct device_header header;
/*   4 0x04 */    i8 signal;
/*   5 0x05 */    const i8 request;
/*   6 0x06 */    const i8 requestStatus;
/*   7 0x07 */    const i8 cooldown;
/*   8 0x08 */    const i32 batteryCapacity;
/*  12 0x0c */    const i32 batteryEnergy;
/*  16 0x10 */    const i32 failReason;
};

#define ROBOT_SIGNAL_MOVE_FORWARD 1
#define ROBOT_SIGNAL_MOVE_BACK 2
#define ROBOT_SIGNAL_ROTATE_LEFT 3
#define ROBOT_SIGNAL_ROTATE_RIGHT 4
#define ROBOT_SIGNAL_ROTATE_UP 5
#define ROBOT_SIGNAL_ROTATE_DOWN 6
#define ROBOT_SIGNAL_MINE_BLOCK 7

#define ROBOT_REQUEST_STATUS_PENDING 0
#define ROBOT_REQUEST_STATUS_RUNNING 1
#define ROBOT_REQUEST_STATUS_FAILED 2
#define ROBOT_REQUEST_STATUS_SUCCESSFUL 3

#define ROBOT_NO_FAIL = 0
#define ROBOT_FAIL_NO_ENERGY = 1
#define ROBOT_FAIL_BLOCKED = 2          // cannot move
#define ROBOT_FAIL_UNBREAKABLE = 3      // cannot mine
#define ROBOT_FAIL_LIMIT_REACHED = 4    // unable to rotate up/down
#define ROBOT_FAIL_INVENTORY_FULL = 5   // unable to store mined block in inventory
#define ROBOT_FAIL_AIR = 6              // try to mine air

#endif //MAGNETICRAFTCOMPUTER_ROBOT_H
