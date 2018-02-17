//
// Created by cout970 on 2017-08-23.
//

#ifndef DRIVER_ROBOT_H
#define DRIVER_ROBOT_H

#include "devices.h"

struct mining_robot_header {
/*   0 0x00 */    struct device_header header;
/*   4 0x04 */    Byte signal;
/*   5 0x05 */    const Byte requestStatus;     // State of the task
/*   6 0x06 */    const Byte cooldown;          // Ticks remaining to complete the task
/*   7 0x07 */    const Byte orientation;       // Orientation of the robot
/*   8 0x08 */    const Int batteryCapacity;    // Max amount of Joules that the battery supports
/*  12 0x0c */    const Int batteryEnergy;      // Amount of Joules in the battery
/*  16 0x10 */    const Int failReason;         // Reason why last task failed
/*  20 0x14 */    const Int scanResult;         // 1 if there is a block on front, 0 otherwise
};

typedef struct mining_robot_header MiningRobot;

#define ROBOT_ORIENTATION_FACING_MASK 0b0011
#define ROBOT_ORIENTATION_LEVEL_MASK 0b1100

#define ROBOT_SIGNAL_MOVE_FORWARD 1
#define ROBOT_SIGNAL_MOVE_BACK 2
#define ROBOT_SIGNAL_ROTATE_LEFT 3
#define ROBOT_SIGNAL_ROTATE_RIGHT 4
#define ROBOT_SIGNAL_ROTATE_UP 5
#define ROBOT_SIGNAL_ROTATE_DOWN 6
#define ROBOT_SIGNAL_MINE_BLOCK 7
#define ROBOT_SIGNAL_SCAN 8

#define ROBOT_REQUEST_STATUS_RUNNING 0
#define ROBOT_REQUEST_STATUS_FAILED 1
#define ROBOT_REQUEST_STATUS_SUCCESSFUL 2

#define ROBOT_NO_FAIL 0
#define ROBOT_FAIL_NO_ENERGY 1
#define ROBOT_FAIL_BLOCKED 2          // cannot move
#define ROBOT_FAIL_UNBREAKABLE 3      // cannot mine
#define ROBOT_FAIL_LIMIT_REACHED 4    // unable to rotate up/down
#define ROBOT_FAIL_INVENTORY_FULL 5   // unable to store mined block in inventory
#define ROBOT_FAIL_AIR 6              // try to mine air

enum Facing {
    FC_SOUTH, FC_WEST, FC_NORTH, FC_EAST
};

enum Level {
    LV_UP, LV_CENTER, LV_DOWN
};

Int mining_robot_signal(MiningRobot *robot, Int signal);

// Return TRUE if there is a block on front, FALSE if there is no block
Boolean mining_robot_scan(MiningRobot *robot);

enum Facing mining_robot_get_facing(MiningRobot *robot);

enum Level mining_robot_get_level(MiningRobot *robot);

Int mining_robot_get_battery_capacity(MiningRobot *robot);

Int mining_robot_get_battery_energy(MiningRobot *robot);

#endif //DRIVER_ROBOT_H
