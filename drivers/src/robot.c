//
// Created by cout970 on 12/02/18.
//

#include "robot.h"
#include <motherboard.h>

Int mining_robot_signal(MiningRobot *rb, Int signal) {
    MiningRobot volatile* robot = rb;

    if (robot->requestStatus != ROBOT_REQUEST_STATUS_SUCCESSFUL &&
        robot->requestStatus != ROBOT_REQUEST_STATUS_FAILED) {
        return -1;
    }
    robot->signal = (Byte) signal;

    if (robot->requestStatus == ROBOT_REQUEST_STATUS_FAILED) {
        return robot->failReason;
    }

    if (robot->requestStatus == ROBOT_REQUEST_STATUS_RUNNING) {
        motherboard_sleep(robot->cooldown);
    }

    if (robot->requestStatus == ROBOT_REQUEST_STATUS_SUCCESSFUL) {
        return ROBOT_NO_FAIL;
    } else {
        return robot->failReason;
    }
}

Boolean mining_robot_scan(MiningRobot *rb) {
    MiningRobot volatile* robot = rb;
    // this cannot fail and runs sync
    robot->signal = ROBOT_SIGNAL_SCAN;
    return robot->scanResult == 1;
}

enum Facing mining_robot_get_facing(MiningRobot *robot) {
    return (enum Facing) (robot->orientation & ROBOT_ORIENTATION_FACING_MASK);
}

enum Level mining_robot_get_level(MiningRobot *robot) {
    return (enum Level) ((robot->orientation & ROBOT_ORIENTATION_LEVEL_MASK) >> 2);
}

Int mining_robot_get_battery_capacity(MiningRobot *robot) {
    return robot->batteryCapacity;
}

Int mining_robot_get_battery_energy(MiningRobot *robot) {
    return robot->batteryEnergy;
}