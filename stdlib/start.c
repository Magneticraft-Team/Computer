//
// Created by cout970 on 2017-07-27.
//

#include "dependencies.h"
#include "api/stdio.h"

i32 fib(i32 pos) {
    if (pos == 0 || pos == 1) return 1;
    return fib(pos - 2) + fib(pos - 1);
}

void test_speed(i32 size) {
    i32 ticks = motherboard_get_minecraft_world_time();
    i32 cycles = motherboard_get_cpu_cycles();
    i32 value = fib(size);
    i32 finalCycles = motherboard_get_cpu_cycles();
    printf("Fib(%d) = %d, time = %d (cycles), %d (ticks)\n", size, value, finalCycles - cycles,
           motherboard_get_minecraft_world_time() - ticks);
}

void test_speed_up_to(i32 num) {
    i32 i;
    for (i = 1; i < num; i++) {
        test_speed(i);
    }
}

// TODO make tests for stdlib
int main() {
    clear_screen();
    test_speed_up_to(35);
    return 0;
}