//
// Created by cout970 on 2017-07-10.
//

//#define USE_DEBUG_LOG

#include <motherboard.h>
#include <network.h>
#include "debug.h"
#include "fs/filesystem.h"

void run_tests();

void print_info();

void test_fs();

void clear_monitor(Monitor *mon);

void test_network_pastebin(NetworkCard *, INodeRef);

static NetworkCard *net = NULL;

void main() {

#ifdef USE_DEBUG_LOG
    motherboard_set_debug_log_type(MOTHERBOARD_LOG_TYPE_CHAR);
#else
    clear_monitor(motherboard_get_monitor());
#endif

    run_tests();
    print_info();
    test_fs();
    if (net) {
        test_network_pastebin(net, 2);
    }
}

void assert(Int expected, Int value, const char *msg) {
    if (expected != value) {
        kdebug("Error: Expected: %x, but got: %x, %s\n", expected, value, msg);
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
    struct network_card_header *mb = 0x0;
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

void clear_monitor(Monitor *mon) {
    Int i;
    Int lines = monitor_get_num_lines(mon);
    Int columns = monitor_get_num_columns(mon);

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
    Motherboard *mb = motherboard_get_computer_motherboard();

    kdebug("Motherboard state:\n");
    kdebug("\tOnline: %d\n", mb->online);
    kdebug("\tMonitorId: %x\n", (int) mb->monitor);
    kdebug("\tFloppyId: %x\n", (int) mb->floppy);
    kdebug("\tMemSize: %d\n", mb->memSize);
    kdebug("\tLittleEndian: %d\n", mb->littleEndian);
    kdebug("\tWorldTime: %d\n", mb->worldTime);
    kdebug("\tCpuTime: %d\n", mb->cpuTime);
}

void print_monitor_data(Monitor *mon) {
    kdebug("\tSize: %d x %d\n", mon->columns, mon->lines);
    kdebug("\tCursor pos: %d, %d\n", mon->cursorColumn, mon->cursorLine);
    kdebug("\tCurrent line: %d\n", mon->currentLine);
}

void print_disk_drive_data(DiskDrive *drive) {
    kdebug("\tTask: %d\n", drive->signal);
    kdebug("\tHas disk: %d\n", drive->hasDisk);
    kdebug("\tAccess time: %d\n", drive->accessTime);
    kdebug("\tSectors: %d\n", drive->numSectors);
    kdebug("\tCurrent sector: %d\n", drive->currentSector);
}

void print_network_data(NetworkCard *net) {
    kdebug("\tInternet Allowed: %d\n", net->internetAllowed);
    kdebug("\tActive Sockets: %d\n", net->activeSockets);
    kdebug("\tMax sockets: %d\n", net->maxSockets);
    kdebug("\tMAC: %x\n", net->macAddress);
    kdebug("\tConnection open: %d\n", net->connectionOpen);
    kdebug("\tConnection error: %d\n", net->connectionError);
    kdebug("\tTarget port: %d\n", net->targetPort);
    kdebug("\tTarget IP: '%s'\n", net->targetIp);
    kdebug("\tInput buffer index: %d\n", net->inputBufferPtr);
    kdebug("\tOutput buffer index: %d\n", net->outputBufferPtr);
    kdebug("\tInput buffer: %x\n", (int) net->inputBuffer);
    kdebug("\tOutput buffer: %x\n", (int) net->outputBuffer);
}

void test_network_pastebin(NetworkCard *net, INodeRef file) {
    int i;

    kdebug("Connecting to pastebin.com...\n");
    network_set_target_ip(net, "pastebin.com");
    network_set_target_port(net, 443);
    network_set_input_pointer(net, 0);
    network_set_output_pointer(net, 0);
    network_signal(net, NETWORK_SIGNAL_OPEN_SSL_TCP_CONNECTION);

    const char *get = "GET /raw/pJwsc2XP HTTP/1.1\r\n"
            "Host: pastebin.com\r\n"
            "Connection: close\r\n"
            "\r\n";

    Byte volatile *ptr = network_get_output_buffer(net);

    for (i = 0; get[i]; i++) {
        ptr[i] = get[i];
    }
    network_set_output_pointer(net, i);

    // Wait for data to be send
    while (network_get_output_pointer(net) && network_is_connection_open(net)) {
        motherboard_sleep(1);
    }

    int offset = 0;
    // wait for response to arrive and print it
    while (network_is_connection_open(net)) {

        while (!network_get_input_pointer(net) && network_is_connection_open(net)) {
            motherboard_sleep(1);
        }
        offset += fs_write(file, network_get_input_buffer(net), offset, network_get_input_pointer(net));
        network_set_input_pointer(net, 0);
    }

    network_signal(net, NETWORK_SIGNAL_CLOSE_TCP_CONNECTION);
    kdebug("Connection closed\n");
    kdebug("Free blocks: %d, total blocks: %d\n", fs_getFreeBlocks(),
           disk_drive_get_num_sectors(motherboard_get_floppy_drive()));
}

void run_tests() {
    kdebug("Starting tests\n");
    test_motherboard();
    text_monitor();
    test_disk_drive();
    test_network();
    kdebug("All tests passed\n");
}

void print_info() {
    int count = 0, i;

    print_motherboard_data();

    kdebug("Scanning devices... \n");
    for (i = 0; i < 16; i++) {
        const struct device_header *h = motherboard_get_devices()[i];
        if (h) {
            kdebug("Found device at %x, of type: %d\n", (unsigned int) h, h->type);
            count++;
            if (h->type == DEVICE_TYPE_NETWORK_CARD) {
                net = (NetworkCard *) h;
            }
        }
    }
    kdebug("All devices scanned, count: %d \n", count);

    kdebug("\nPrinting monitor data...\n");
    Monitor *mon = motherboard_get_monitor();
    if (mon != NULL) {
        print_monitor_data(mon);
    } else {
        kdebug("No monitor found\n");
    }

    kdebug("\nPrinting floppy disk data...\n");
    DiskDrive *drive = motherboard_get_floppy_drive();
    if (drive != NULL) {
        fs_init(drive);
        print_disk_drive_data(drive);
    } else {
        kdebug("No floppy drive found\n");
    }

    kdebug("\nPrinting network card data...\n");
    if (net != NULL) {
        print_network_data(net);
//        test_network_pastebin(net);
    } else {
        kdebug("No network card found\n");
    }

    kdebug("End\n");
}

void test_ls(INodeRef currentFolder) {

    struct DirectoryIterator iter;
    struct INode node;

    fs_iterInit(currentFolder, &iter);

    while (fs_iterNext(&iter)) {
        kdebug("%s", iter.entry.name);

        // Check if is directory
        fs_getINode(iter.entry.inode, &node);
        if (node.flags == FS_FLAG_DIRECTORY) {
            kdebug("/");
        }
        kdebug("\n");
    }
}

void test_fs() {
    kdebug("Create fs\n");
    fs_format();
    kdebug("Free blocks: %d, total blocks: %d\n", fs_getFreeBlocks(),
           disk_drive_get_num_sectors(motherboard_get_floppy_drive()));
    INodeRef folder = fs_getRoot();
    INodeRef child1 = fs_create(folder, "file.txt", FS_FLAG_FILE);
//    INodeRef child2 = fs_create(folder, "no_file.txt", FS_FLAG_FILE);
//    INodeRef folder2 = fs_create(folder, "stuff", FS_FLAG_DIRECTORY);
//    INodeRef child3 = fs_create(folder2, "file3.txt", FS_FLAG_FILE);
//
//    kdebug("Truncate file3, to size: %d\n", 1024 * 10);
//    fs_truncate(child3, 1024 * 10);
//    kdebug("Truncate file3, to size: 0\n");
//    fs_truncate(child3, 0);
//
//    kdebug("Test file write fs\n");
//    char *text1 = "file1: hola mundo!!";
//    char *text2 = "file2: hello world!!";
//    fs_write(child1, text1, 0, strlen(text1) + 1);
//    fs_write(child2, text2, 0, strlen(text2) + 1);
//    kdebug("\n");
//
//    kdebug("> ls\n");
//    test_ls(folder);
//
//    kdebug("Delete files\n");
//
//    fs_delete(folder, child2);
//    fs_delete(folder, child1);
//    fs_delete(folder, folder2);
//
//    kdebug("> ls\n");
//    test_ls(folder);
//
//    kdebug("Test fill disk\n");
//
//    char buffer[1024];
//    char fileName[] = "a";
//    INodeRef newFile;
//
//    for (int i = 'a'; i <= 'z'; ++i) {
//
//        memset(buffer, (char) i, 1024);
//        fileName[0] = (char) i;
//        newFile = fs_create(folder, fileName, FS_FLAG_FILE);
//        kdebug("New file %d, at '%s'\n", newFile, fileName);
//        fs_write(newFile, buffer, 0, 1024);
//    }
//
//    kdebug("Free blocks: %d, total blocks: %d\n", fs_getFreeBlocks(),
//           disk_drive_get_num_sectors(motherboard_get_floppy_drive()));
//
//    kdebug("> ls\n");
//    test_ls(folder);
//    BlockRef child = fs_findFile(folder, "z");
//    fs_delete(folder, child);
//
//    child = fs_findFile(folder, "t");
//    fs_delete(folder, child);
//
//    kdebug("> ls\n");
//    test_ls(folder);

    kdebug("Free blocks: %d, total blocks: %d\n", fs_getFreeBlocks(),
           disk_drive_get_num_sectors(motherboard_get_floppy_drive()));

    kdebug("End\n");
}
