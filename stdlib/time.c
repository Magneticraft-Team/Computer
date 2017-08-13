//
// Created by cout970 on 2016-10-28.
//

#include "dependencies.h"
#include "api/time.h"

clock_t clock(void) {
    return (clock_t) motherboard_get_minecraft_world_time();
}

unsigned int cpu_clock(void) {
    return (unsigned int) motherboard_get_cpu_cycles();
}