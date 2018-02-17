//
// Created by cout970 on 12/02/18.
//


#include <debug.h>
#include <monitor.h>
#include <fs/filesystem.h>
#include <network.h>
#include <motherboard.h>
#include "../include/quarry.h"

#define CHECK_STOP() if (stopped || checkKey('q')) { stopped = TRUE; return; }

extern struct {
    INodeRef currentFolder;
    Int fsInitialized;
    Int run;
    NetworkCard *networkCard;
    Monitor *monitor;
    DiskDrive *diskDrive;
    MiningRobot *robot;
} state;

typedef struct {
    Int x;
    Int y;
    Int z;
} vec3;

typedef enum {
    DIR_DOWN,
    DIR_UP,
    DIR_NORTH,
    DIR_SOUTH,
    DIR_WEST,
    DIR_EAST
} Direction;

static MiningRobot *rb;
static vec3 pos;
static Boolean stopped;

Boolean checkKey(Char key) {
    while (monitor_has_key_events(state.monitor)) {
        struct key_event e = monitor_get_last_key_event(state.monitor);
        if (e.press && e.character == key) {
            return TRUE;
        }
    }
    return FALSE;
}

void turnLeft() {
    mining_robot_signal(rb, ROBOT_SIGNAL_ROTATE_LEFT);
}

void turnRight() {
    mining_robot_signal(rb, ROBOT_SIGNAL_ROTATE_RIGHT);
}

void rotateAt(enum Level at) {
    if (at == LV_DOWN) {
        do {
            mining_robot_signal(rb, ROBOT_SIGNAL_ROTATE_DOWN);
        } while (mining_robot_get_level(rb) != LV_DOWN);

    } else if (at == LV_UP) {
        do {
            mining_robot_signal(rb, ROBOT_SIGNAL_ROTATE_UP);
        } while (mining_robot_get_level(rb) != LV_UP);

    } else {
        // at is CENTER
        enum Level lv = mining_robot_get_level(rb);
        if (lv == LV_DOWN) {
            mining_robot_signal(rb, ROBOT_SIGNAL_ROTATE_UP);
        } else if (lv == LV_UP) {
            mining_robot_signal(rb, ROBOT_SIGNAL_ROTATE_DOWN);
        }
    }
}

enum Facing turnFacingCW(enum Facing a) {
    switch (a) {
        case FC_SOUTH:
            return FC_WEST;
        case FC_WEST:
            return FC_NORTH;
        case FC_NORTH:
            return FC_EAST;
        default:
            return FC_SOUTH;
    }
}

void faceAt(enum Facing at) {
    rotateAt(LV_CENTER);
    enum Facing now = mining_robot_get_facing(rb);
    if (at == turnFacingCW(now)) {
        turnRight();
    } else if (at == turnFacingCW(turnFacingCW(now))) {
        turnRight();
        turnRight();
    } else if (at != now) {
        turnLeft();
    }
}

void lookAt(Direction at) {
    if (at == DIR_DOWN) {
        rotateAt(LV_DOWN);
    } else if (at == DIR_UP) {
        rotateAt(LV_UP);
    } else {
        if (at == DIR_NORTH) {
            faceAt(FC_NORTH);
        } else if (at == DIR_SOUTH) {
            faceAt(FC_SOUTH);
        } else if (at == DIR_WEST) {
            faceAt(FC_WEST);
        } else {
            faceAt(FC_EAST);
        }
    }
}


Direction getDirection() {
    enum Level lv = mining_robot_get_level(rb);
    if (lv == LV_DOWN) return DIR_DOWN;
    if (lv == LV_UP) return DIR_UP;
    enum Facing fc = mining_robot_get_facing(rb);
    if (fc == FC_NORTH) return DIR_NORTH;
    if (fc == FC_SOUTH) return DIR_SOUTH;
    if (fc == FC_WEST) return DIR_WEST;
    return DIR_EAST;
}

vec3 fromDirection(Direction dir) {
    switch (dir) {
        case DIR_DOWN:
            return (vec3) {.x = 0, .y = -1, .z = 0};
        case DIR_UP:
            return (vec3) {.x = 0, .y = 1, .z = 0};
        case DIR_NORTH:
            return (vec3) {.x = 0, .y = 0, .z = -1};
        case DIR_SOUTH:
            return (vec3) {.x = 0, .y = 0, .z = 1};
        case DIR_WEST:
            return (vec3) {.x = -1, .y = 0, .z = 0};
        default:
            return (vec3) {.x = 1, .y = 0, .z = 0};
    }
}

vec3 add(vec3 a, vec3 b) {
    return (vec3) {.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

vec3 relative(enum Facing dir, vec3 abs) {
    switch (dir) {

        case FC_SOUTH:
            return (vec3) {.x = -abs.z, .y = abs.y, .z = abs.x};
        case FC_WEST:
            return (vec3) {.x = -abs.x, .y = abs.y, .z = -abs.z};
        case FC_NORTH:
            return (vec3) {.x = abs.z, .y = abs.y, .z = -abs.x};
        default:
            return abs;
    }
}

void moveForward() {
    if (mining_robot_signal(rb, ROBOT_SIGNAL_MOVE_FORWARD) == ROBOT_NO_FAIL) {
        vec3 a = fromDirection(getDirection());
        pos = add(pos, a);
    }
}

void mineWell() {
    Int height = 0, code = 0;
    kdebug("Mining down...\n");
    lookAt(DIR_DOWN);
    while (1) {
        CHECK_STOP();

        if (mining_robot_scan(rb)) {
            code = mining_robot_signal(rb, ROBOT_SIGNAL_MINE_BLOCK);
            if (code == ROBOT_FAIL_UNBREAKABLE) {
                kdebug("Unable to break block, assuming it's bedrock\n");
                break;
            } else if (code == ROBOT_FAIL_INVENTORY_FULL) {
                kdebug("Inventory full, stopping\n");
                return;
            } else if (code == ROBOT_FAIL_NO_ENERGY) {
                kdebug("Run out of energy, stopping\n");
                return;
            }
        }

        code = mining_robot_signal(rb, ROBOT_SIGNAL_MOVE_FORWARD);
        if (code == ROBOT_NO_FAIL) {
            height++;
        } else if (!mining_robot_scan(rb) && code == ROBOT_FAIL_BLOCKED) {
            kdebug("Unable to move, there is nothing on front, assuming it's the void\n");
            break;
        }
        kdebug("level: %d\n", height);
    }

    kdebug("returning...\n");
    mining_robot_signal(rb, ROBOT_SIGNAL_ROTATE_UP);
    mining_robot_signal(rb, ROBOT_SIGNAL_ROTATE_UP);

    for (int i = 0; i < height; ++i) {
        CHECK_STOP();

        if (mining_robot_scan(rb) == 1) {
            mining_robot_signal(rb, ROBOT_SIGNAL_MINE_BLOCK);
        }
        while (mining_robot_signal(rb, ROBOT_SIGNAL_MOVE_FORWARD) != ROBOT_NO_FAIL);
        kdebug("level: %d\n", height - i);
    }
    lookAt(DIR_UP);
    kdebug("Done\n");
}

void moveTo(vec3 newPos) {
    kdebug("Pos {x = %d, y = %d, z = %d}\n", pos.x, pos.y, pos.z);
    kdebug("Moving to {x = %d, y = %d, z = %d}\n", newPos.x, newPos.y, newPos.z);

    if (pos.y > newPos.y) {
        lookAt(DIR_DOWN);
        do {
            moveForward();
            CHECK_STOP();
        } while (pos.y > newPos.y);

    } else if (pos.y < newPos.y) {
        lookAt(DIR_UP);
        do {
            moveForward();
            CHECK_STOP();
        } while (pos.y < newPos.y);
    }

    if (pos.x > newPos.x) {
        lookAt(DIR_WEST);
        do {
            moveForward();
            CHECK_STOP();
        } while (pos.x > newPos.x);

    } else if (pos.x < newPos.x) {
        lookAt(DIR_EAST);
        do {
            moveForward();
            CHECK_STOP();
        } while (pos.x < newPos.x);
    }

    if (pos.z > newPos.z) {
        lookAt(DIR_NORTH);
        do {
            moveForward();
            CHECK_STOP();
        } while (pos.z > newPos.z);

    } else if (pos.z < newPos.z) {
        lookAt(DIR_SOUTH);
        do {
            moveForward();
            CHECK_STOP();
        } while (pos.z < newPos.z);
    }
}

void quarry_start(MiningRobot *_rb, Int size) {
    rb = _rb;
    stopped = FALSE;
    kdebug("Press q to stop\n");
    enum Facing initialD = mining_robot_get_facing(rb);
    pos = (vec3) {.x = 0, .y = 0, .z = 0};

    for (int x = 0; x < size; ++x) {
        for (int z = 0; z < size; ++z) {
            moveTo(relative(initialD, (vec3) {.x = x, .y = 0, .z = z}));
            CHECK_STOP();
            mineWell();
            CHECK_STOP();
            moveTo((vec3) {.x = 0, .y = 0, .z = 0});
            CHECK_STOP();
            kdebug("Waiting 5 seconds for inventory unload\n");
            motherboard_sleep(20 * 5);
            kdebug("Go\n");
        }
    }

    kdebug("Finished\n");
}