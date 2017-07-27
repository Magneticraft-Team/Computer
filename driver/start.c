//
// Created by cout970 on 2017-07-10.
//

//#define USE_DEBUG_LOG

#include "api/motherboard.h"
#include "api/monitor.h"
#include "api/disk_drive.h"
#include "printf.h"
#include "api/network.h"

void run_tests();
void print_info();

void clear_monitor(Monitor mon);

void main() {

#ifdef USE_DEBUG_LOG
    motherboard_set_debug_log_type(MOTHERBOARD_LOG_TYPE_CHAR);
#else
    clear_monitor((Monitor) motherboard_get_monitor());
#endif

    run_tests();
    print_info();
}

void assert(i32 expected, i32 value, const char *msg) {
    if (expected != value) {
        printf("Error: Expected: %x, but got: %x, %s\n", expected, value, msg);
        motherboard_signal(MOTHERBOARD_SIGNAL_HALT);
    }
}

void test_motherboard() {
    struct motherboard_header *mb = 0x0;
    assert(0x00, (int) &mb->online, "online");
    assert(0x01, (int) &mb->signal, "signal");
    assert(0x02, (int) &mb->sleep, "sleep");
    assert(0x03, (int) &mb->padding, "padding");
    assert(0x04, (int) &mb->memSize, "memSize");
    assert(0x08, (int) &mb->littleEndian, "littleEndian");
    assert(0x0c, (int) &mb->worldTime, "worldTime");
    assert(0x10, (int) &mb->cpuTime, "cpuTime");
    assert(0x14, (int) &mb->logType, "logType");
    assert(0x15, (int) &mb->logByte, "logByte");
    assert(0x16, (int) &mb->logShort, "logShort");
    assert(0x18, (int) &mb->logInt, "logInt");
    assert(0x1c, (int) &mb->monitor, "monitor");
    assert(0x20, (int) &mb->floppy, "floppy");
    assert(0x24, (int) &mb->devices, "devices");
}

void text_monitor() {
    struct monitor_header *mb = 0x0;
    assert(0x04, (int) &mb->keyBufferPtr, "keyBufferPtr");
    assert(0x05, (int) &mb->keyBufferSize, "keyBufferSize");
    assert(0x06, (int) &mb->keyBuffer, "keyBuffer");
    assert(0x22, (int) &mb->mouseBufferPtr, "mouseBufferPtr");
    assert(0x23, (int) &mb->mouseBufferSize, "mouseBufferSize");
    assert(0x24, (int) &mb->mouseBuffer, "mouseBuffer");
    assert(0x48, (int) &mb->lines, "lines");
    assert(0x4c, (int) &mb->columns, "columns");
    assert(0x50, (int) &mb->cursorLine, "cursorLine");
    assert(0x54, (int) &mb->cursorColumn, "cursorColumn");
    assert(0x58, (int) &mb->signal, "signal");
    assert(0x5a, (int) &mb->currentLine, "currentLine");
    assert(0x5c, (int) &mb->buffer, "buffer");
}

void test_disk_drive() {
    struct disk_drive_header *mb = 0x0;
    assert(0x00, (int) &mb->header, "header");
    assert(0x04, (int) &mb->signal, "signal");
    assert(0x05, (int) &mb->hasDisk, "hasDisk");
    assert(0x06, (int) &mb->accessTime, "accessTime");
    assert(0x07, (int) &mb->padding, "padding");
    assert(0x08, (int) &mb->numSectors, "numSectors");
    assert(0x0c, (int) &mb->currentSector, "currentSector");
    assert(0x10, (int) &mb->buffer, "buffer");
}

void test_network() {
    struct network_header *mb = 0x0;
    assert(0x000, (int) &mb->header, "header");
    assert(0x004, (int) &mb->internetAllowed, "internetAllowed");
    assert(0x005, (int) &mb->maxSockets, "maxSockets");
    assert(0x006, (int) &mb->activeSockets, "activeSockets");
    assert(0x007, (int) &mb->signal, "signal");
    assert(0x008, (int) &mb->macAddress, "macAddress");
    assert(0x00c, (int) &mb->targetMac, "targetMac");
    assert(0x010, (int) &mb->targetPort, "targetPort");
    assert(0x014, (int) &mb->targetIp, "targetIp");
    assert(0x064, (int) &mb->connectionOpen, "connectionOpen");
    assert(0x068, (int) &mb->connectionError, "connectionError");
    assert(0x06c, (int) &mb->inputBufferPtr, "inputBufferPtr");
    assert(0x070, (int) &mb->outputBufferPtr, "outputBufferPtr");
    assert(0x074, (int) &mb->inputBuffer, "inputBuffer");
    assert(0x474, (int) &mb->outputBuffer, "outputBuffer");
}

void clear_monitor(Monitor mon) {
    i32 i;
    i32 lines = monitor_get_num_lines(mon);
    i32 columns = monitor_get_num_columns(mon);

    for (i = 0; i < columns; i++) {
        monitor_get_line_buffer(mon)[i] = 0x20;
    }

    for (i = 0; i < lines; i++) {
        monitor_set_selected_line(mon, i);
        monitor_signal(mon, MONITOR_SIGNAL_WRITE);
    }
    monitor_set_selected_line(mon, 0);
}

void print_motherboard_data() {
    i32 i;
    struct motherboard_header *mb = (struct motherboard_header *) 0xFFFF0000;

    printf("All tests passed\n");
    printf("Online: %d\n", mb->online);
    printf("MonitorId: %d\n", (int) mb->monitor);
    printf("FloppyId: %d\n", (int) mb->floppy);
    for (i = 0; i < 16; i++)
        printf("Device %d: %d\n", i, (int) mb->devices[i]);
    printf("MemSize: %d\n", mb->memSize);
    printf("LittleEndian: %d\n", mb->littleEndian);
    printf("WorldTime: %d\n", mb->worldTime);
    printf("CpuTime: %d\n", mb->cpuTime);
}

void print_monitor_data(Monitor mon) {
    printf("Size: %d x %d\n", mon->columns, mon->lines);
    printf("Cursor pos: %d, %d\n", mon->cursorColumn, mon->cursorLine);
    printf("Current line: %d\n", mon->currentLine);
}

void print_disk_drive_data(DiskDrive drive) {
    printf("Task: %d\n", drive->signal);
    printf("Has disk: %d\n", drive->hasDisk);
    printf("Access time: %d\n", drive->accessTime);
    printf("Sectors: %d\n", drive->numSectors);
    printf("Current sector: %d\n", drive->currentSector);
}

void print_network_data(Network net) {
    printf("Internet Allowed: %d\n", net->internetAllowed);
    printf("Active Sockets: %d\n", net->activeSockets);
    printf("Max sockets: %d\n", net->maxSockets);
    printf("MAC: %x\n", net->macAddress);
    printf("Connection open: %d\n", net->connectionOpen);
    printf("Connection error: %d\n", net->connectionError);
    printf("Target port: %d\n", net->targetPort);
    printf("Target IP: '%s'\n", net->targetIp);
    printf("Input buffer pointer: %d\n", net->inputBufferPtr);
    printf("Output buffer pointer: %d\n", net->outputBufferPtr);
}

//void memcpy(void *src, void *dst, i32 size) {
//    i32 i;
//    for (i = 0; i < size; i++) {
//        ((i8 *) dst)[i] = ((i8 *) src)[i];
//    }
//}
//
//void test_network_pastebin(Network net) {
//    network_set_target_ip(net, "pastebin.com");
//    network_set_target_port(net, 80);
//    network_signal(net, NETWORK_SIGNAL_OPEN_TCP_CONNECTION);
//    network_set_input_pointer(net, 0);
//    network_set_output_pointer(net, 0);
//
//    const char *get = "GET /raw/zvbgT1fw HTTP/1.0\nHost: pastebin.com\n\n";
//    printf("Petition: '%s'\n", get);
//    i8 *ptr = network_get_output_buffer(net);
//    memcpy((void *) get, ptr, 48);
//    network_set_output_pointer(net, 48);
//
//    while (network_get_output_pointer(net) && network_is_connection_open(net)) {
//        motherboard_sleep(1);
//    }
//
//    while (!network_get_input_pointer(net) && network_is_connection_open(net)) {
//        motherboard_sleep(1);
//    }
//    printf("Response: '%s' (%d)\n", network_get_input_buffer(net), network_get_input_pointer(net));
//    network_signal(net, NETWORK_SIGNAL_CLOSE_TCP_CONNECTION);
//}
//
//i32 fib(i32 pos) {
//    if (pos == 0 || pos == 1) return 1;
//    return fib(pos - 2) + fib(pos - 1);
//}
//
//void test_speed(i32 size) {
//    i32 ticks = motherboard_get_minecraft_world_time();
//    i32 cycles = motherboard_get_cpu_cycles();
//    i32 value = fib(size);
//    i32 finalCycles = motherboard_get_cpu_cycles();
//    printf("Fib(%d) = %d, time = %d (cycles), %d (ticks)\n", size, value, finalCycles - cycles,
//           motherboard_get_minecraft_world_time() - ticks);
//}
//
//void test_speed_up_to(i32 num) {
//    i32 i;
//    for (i = 1; i < num; i++) {
//        test_speed(i);
//    }
//}

void run_tests() {
    printf("Starting tests\n");
    test_motherboard();
    text_monitor();
    test_disk_drive();
    test_network();
    printf("All tests passed\n");
}

void print_info() {
    i32 i;
    printf("Scaning devices.. \n");
    for (i = 0; i < 16; i++) {
        struct device_header *h = motherboard_get_devices()[i];
        if (h) {
            printf("Found device at %x, of type: %d\n", (unsigned int) h, h->type);
        }
    }

    printf("\nPrinting monitor data...\n");
    Monitor mon = (Monitor) motherboard_get_monitor();
    print_monitor_data(mon);

    printf("\nPrinting floppy disk data...\n");
    DiskDrive drive = (DiskDrive) motherboard_get_floppy_drive();
    print_disk_drive_data(drive);

    printf("\nPrinting network card data...\n");
    Network net = (Network) motherboard_get_devices()[2];
    print_network_data(net);

    printf("End\n");
}