//
// Created by cout970 on 10/02/18.
//

#include <types.h>
#include <debug.h>
#include <motherboard.h>
#include <fs/filesystem.h>
#include <glib/ctype.h>
#include <fs/access.h>
#include <util/bitmap.h>
#include <network.h>
#include <string.h>
#include <glib/math.h>
#include <robot.h>
#include <kprint.h>
#include "../include/input.h"
#include "../include/quarry.h"
#include "../include/commands.h"


struct Cmd commands[] = {
        {.name = "help", .argCount = 0, .func0 = f_help, .usage = "Prints help"},
        {.name = "ls", .argCount = 0, .func0 = f_ls, .usage = "List directory contents"},
        {.name = "cd", .argCount = 1, .func1 = f_cd, .usage = "Change directory"},
        {.name = "mkdir", .argCount = 1, .func1 = f_mkdir, .usage = "Create a directory"},
        {.name = "rm", .argCount = 1, .func1 = f_rm, .usage = "Remove a file/directory"},
        {.name = "format", .argCount = 0, .func0 = f_format, .usage = "Format current disk"},
        {.name = "free", .argCount = 0, .func0 = f_free, .usage = "Show free space"},
        {.name = "fs", .argCount = 0, .func0 = f_fs, .usage = "Show filesystem data"},
        {.name = "cat", .argCount = 1, .func1 = f_cat, .usage = "Show file contents"},
        {.name = "touch", .argCount = 1, .func1 = f_touch, .usage = "Create a empty file"},
        {.name = "write", .argCount = 1, .func1 = f_write, .usage = "Write text to a file"},
        {.name = "update_disk", .argCount = 0, .func0 = f_update_disk, .usage = "Update disk cache"},
        {.name = "pastebin", .argCount = 2, .func2 = f_pastebin, .usage = "Get data from pastebin, args: outputFile, code"},
        {.name = "update", .argCount = 1, .func1 = f_update, .usage = "Downloads a new version of software"},
        {.name = "quarry", .argCount = 1, .func1 = f_quarry, .usage = "Mine a NxN area down to bedrock"},
};

Int commandCount = (Int) (sizeof(commands) / sizeof(struct Cmd));

struct {
    INodeRef currentFolder;
    Int fsInitialized;
    Int run;
    NetworkCard *networkCard;
    Monitor *monitor;
    DiskDrive *diskDrive;
    MiningRobot *robot;
} state;

static void searchPeripherals() {
    const struct device_header **a = motherboard_get_devices();
    state.diskDrive = motherboard_get_floppy_drive();
    state.monitor = motherboard_get_monitor();
    state.networkCard = NULL;
    state.robot = NULL;

    for (int i = 0; i < MOTHERBOARD_MAX_DEVICES; ++i) {
        if (a[i] && a[i]->online) {
            if (a[i]->type == DEVICE_TYPE_NETWORK_CARD) {
                state.networkCard = (NetworkCard *) a[i];
            } else if (a[i]->type == DEVICE_TYPE_MINING_ROBOT) {
                state.robot = (MiningRobot *) a[i];
            }
        }
    }
}

void main() {
    searchPeripherals();
    fs_init(state.diskDrive);
    state.currentFolder = fs_getRoot();
    state.fsInitialized = fs_getDevice() != -1;
    state.run = 1;
    monitor_clear(state.monitor);

    while (state.run) {
        String input[78];
        String cmd[78];
        String arg1[78];
        String arg2[78];
        String arg3[78];

        kdebug("> ");
        readInput(input, sizeof(input));
        Int args = split(input, cmd, arg1, arg2, arg3);
        Int found = 0;

        if (*cmd == '\0') continue;

        for (int i = 0; i < commandCount; ++i) {
            if (strmatch(commands[i].name, cmd)) {
                found = 1;
                if (commands[i].argCount == args) {
                    switch (args) {
                        case 0:
                            commands[i].func0();
                            break;
                        case 1:
                            commands[i].func1(arg1);
                            break;
                        case 2:
                            commands[i].func2(arg1, arg2);
                            break;
                        case 3:
                            commands[i].func3(arg1, arg2, arg3);
                            break;
                        default:
                            kdebug("Invalid arg count, %d\n", args);
                    }
                    break;
                } else {
                    kdebug("Incorrect arg count for '%s', expected %d, got %d\n", cmd, commands[i].argCount, args);
                    break;
                }
            }
        }
        if (!found) {
            kdebug("Command '%s' not found\n", cmd);
        }
    }
}
