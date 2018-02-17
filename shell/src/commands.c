//
// Created by cout970 on 17/02/18.
//

#include <types.h>
#include <debug.h>
#include <motherboard.h>
#include <fs/filesystem.h>
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
#include "../include/commands.h"

#define CHECK_FS() if(!state.fsInitialized){ kdebug("Disk not formatted\n"); return; }

extern struct Cmd commands[];
extern Int commandCount;

extern struct {
    INodeRef currentFolder;
    Int fsInitialized;
    Int run;
    NetworkCard *networkCard;
    Monitor *monitor;
    DiskDrive *diskDrive;
    MiningRobot *robot;
} state;

void f_help() {
    Int usageSize = 0;
    kdebug("Commands:\n");
    for (int i = 0; i < commandCount; ++i) {
        kdebug(" %s: { args: %d, usage: '%s'} \n", commands[i].name, commands[i].argCount, commands[i].usage);
        usageSize = MAX(usageSize, (Int) strlen(commands[i].usage));
    }
    kdebug("[DEBUG] %d\n", usageSize);
}

void f_format() {
    fs_format();
    state.fsInitialized = fs_getDevice() != -1;
}

void f_ls() {
    fs_init(state.diskDrive);
    state.fsInitialized = fs_getDevice() != -1;
    CHECK_FS();
    struct DirectoryIterator iter;
    struct INode node;

    fs_iterInit(state.currentFolder, &iter);

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

void f_cd(ArgType name) {
    CHECK_FS();
    INodeRef child = fs_findFile(state.currentFolder, name);
    if (child == FS_NULL_INODE_REF) {
        kdebug("Error %s doesn't exist\n", name);
        return;
    }
    struct INode node;
    fs_getINode(child, &node);
    if (node.flags == FS_FLAG_DIRECTORY) {
        state.currentFolder = child;
        return;
    } else {
        kdebug("Error %s is not a folder\n", name);
        return;
    }
}

void f_mkdir(ArgType name) {
    CHECK_FS();
    INodeRef res = fs_create(state.currentFolder, name, FS_FLAG_DIRECTORY);
    if (res == FS_NULL_INODE_REF) {
        kdebug("Error unable to create '%s'\n", name);
    }
}

void f_rm(ArgType name) {
    CHECK_FS();
    INodeRef child = fs_findFile(state.currentFolder, name);
    if (child == FS_NULL_INODE_REF) {
        kdebug("Error %s not found\n", name);
        return;
    }

    if (fs_delete(state.currentFolder, child)) {
        kdebug("Error unable to remove file '%s'\n", name);
        return;
    }
}

void f_free() {
    CHECK_FS();
    Int totalBlocks = disk_drive_get_num_sectors(state.diskDrive);
    kdebug("Free blocks: %d, total blocks: %d\n", fs_getFreeBlocks(), totalBlocks);
}

void f_cat(ArgType name) {
    CHECK_FS();
    INodeRef file = fs_findFile(state.currentFolder, name);
    if (file == FS_NULL_INODE_REF) {
        kdebug("Error file '%s' not found\n", name);
        return;
    }

    String line[80];
    Int index = 0, read;
    while (1) {
        read = fs_read(file, line, index, sizeof(line) - 1);
        if (read == 0) return;
        line[read] = '\0';
        index += read;

        kdebug("%s", line);
    }
}

void f_touch(ArgType name) {
    CHECK_FS();
    INodeRef res = fs_create(state.currentFolder, name, FS_FLAG_FILE);
    if (res == FS_NULL_INODE_REF) {
        kdebug("Error unable to create '%s'\n", name);
    }
}

void f_write(ArgType name) {
    CHECK_FS();
    INodeRef file = fs_findFile(state.currentFolder, name);
    if (file == FS_NULL_INODE_REF) {
        file = fs_create(state.currentFolder, name, FS_FLAG_FILE);
        if (file == FS_NULL_INODE_REF) {
            kdebug("Error unable to create '%s'\n", name);
            return;
        }
    }
    String line[78];
    Int index = 0;
    fs_truncate(file, 0);
    kdebug("Type 'EOF' to exit\n");
    while (1) {
        kdebug(">>");
        readInput(line, 78);
        if (strmatch(line, "EOF\n")) return;
        fs_write(file, line, index, strlen(line));
        index += strlen(line);
    }
}

void f_update_disk() {
    state.diskDrive = motherboard_get_floppy_drive();
    fs_init(state.diskDrive);
    state.fsInitialized = fs_getDevice() != -1;
}

void f_fs() {
    loadSector(0);
    BlockRef inodeTableRef = -1;
    struct SuperBlock *superBlock = (struct SuperBlock *) blockBuffer;
    kdebug("SuperBlock: {\n");
    kdebug(" magicNumber: %x,\n", superBlock->magicNumber);
    kdebug(" deviceId: %d,\n", superBlock->deviceId);
    kdebug(" deviceSize: %d,\n", superBlock->deviceSize);
    kdebug(" numGroups: %d,\n", superBlock->numGroups);
    kdebug(" nextBlockGroupList: %d,\n", superBlock->nextBlockGroupList);
    kdebug(" blockGroupList: [\n");
    for (int i = 0; i < superBlock->numGroups; ++i) {
        struct BlockGroup *group = &superBlock->blockGroupList[i];
        kdebug("  BlockGroup: {\n");
        kdebug("  numberOfBlocks: %d\n", group->numberOfBlocks);
        kdebug("  blockBitmap: ");
        for (int j = 0; j < FS_BLOCK_GROUP_BITMAP_BYTES * 8; ++j) {
            kdebug("%d", bitmap_get(group->blockBitmap, j) ? 1 : 0);
        }
        kdebug(",\n");
        kdebug("  blocksOffset: %d\n", group->blocksOffset);
        kdebug("  inodeTable: %d\n", group->inodeTable);
        kdebug("  }\n");
        inodeTableRef = group->inodeTable;
    }
    kdebug(" ]\n");
    kdebug("}\n");
    while (inodeTableRef != -1) {
        loadSector(inodeTableRef);
        struct INodeTable *inodeTable = (struct INodeTable *) blockBuffer;
        kdebug("INodeTable: {\n");
        kdebug(" nextTable: %d,\n", inodeTable->nextTable);
        kdebug(" inodeBitmap: ");
        for (int j = 0; j < FS_INODES_PER_TABLE; ++j) {
            kdebug("%d", bitmap_get((ByteBuffer) &inodeTable->inodeBitmap, j) ? 1 : 0);
        }
        kdebug(",\n");
        kdebug(" inodes: [\n");
        for (int i = 0; i < FS_INODES_PER_TABLE; ++i) {
            if (bitmap_get((ByteBuffer) &inodeTable->inodeBitmap, i)) {
                struct INode *node = &inodeTable->inodes[i];
                kdebug("  {\n");
                kdebug("   flags: %s,\n", (node->flags == FS_FLAG_DIRECTORY) ? "Directory" : "Normal");
                kdebug("   size: %d,\n", node->size);
                kdebug("   accessTime: %d,\n", node->accessTime);
                kdebug("   modificationTime: %d,\n", node->modificationTime);
                kdebug("   creationTime: %d,\n", node->creationTime);
                kdebug("   blocksInUse: %d,\n", node->blocksInUse);
                kdebug("   indirectBlock: %d,\n", node->indirectBlock);
                kdebug("   blocks: [");
                for (int j = 0; j < node->blocksInUse; ++j) {
                    if (j == 0) {
                        kdebug("%d", node->blocks[j]);
                    } else {
                        kdebug(", %d", node->blocks[j]);
                    }
                }
                kdebug("]\n");
                kdebug("  }\n");
            }
        }
        kdebug(" ]\n");
        kdebug("}\n");
        inodeTableRef = inodeTable->nextTable;
    }
}

static Int process_http_response2(NetworkCard *net, void(*func)(Byte)) {
    Byte buffer[80];
    Int read;
    Boolean headerSection = TRUE;
    Int lineSize = 0;
    Int size = 0;

    while (1) {
        read = network_receive(net, buffer, 80);
        if (read != 0) {
            size += read;
            kdebug("Progress: %d Bytes\n", size);
            for (int i = 0; i < read; ++i) {
                if (!headerSection) {
                    func(buffer[i]);
                } else {
                    if (buffer[i] == '\n') {
                        if (lineSize == 1) {
                            headerSection = FALSE;
                        }
                        lineSize = 0;
                    } else {
                        lineSize++;
                    }
                }
            }
        } else {
            if (!network_is_connection_open(net)) break;
            motherboard_sleep(1);
        }
    }
    return size;
}

static Int lambda_arg_offset = 0;
static INodeRef lambda_arg_file = 0;

void lambda_write_to_file(Byte byte) {
    lambda_arg_offset += fs_write(lambda_arg_file, &byte, lambda_arg_offset, 1);
}

void f_pastebin(ArgType name, ArgType code) {
    CHECK_FS();

    NetworkCard *net = state.networkCard;
    if (net == NULL) {
        kdebug("No network card found\n");
        return;
    }

    INodeRef file = fs_findFile(state.currentFolder, name);
    if (file == FS_NULL_INODE_REF) {
        file = fs_create(state.currentFolder, name, FS_FLAG_FILE);
        if (file == FS_NULL_INODE_REF) {
            kdebug("Error unable to create '%s'\n", name);
            return;
        }
    }

    kdebug("Connecting to pastebin.com...\n");
    network_set_target_ip(net, "pastebin.com");
    network_set_target_port(net, 443);
    network_signal(net, NETWORK_SIGNAL_OPEN_SSL_TCP_CONNECTION);

    if (!network_is_connection_open(net)) {
        int errcode = network_get_connection_error(net);
        kdebug("Error connecting to pastebin.com, error code: %d\n", errcode);
        return;
    }
    kdebug("Connected\n");

    // HTTP layer
    String *getPre = "GET /raw/";
    String *getPost = " HTTP/1.0\r\n" //fuck you pastebin
            "Host: pastebin.com\r\n"
            "Connection: close\r\n"
            "\r\n";

    // Send
    network_send(net, getPre, strlen(getPre));
    network_send(net, code, strlen(code));
    network_send(net, getPost, strlen(getPost));

    lambda_arg_offset = 0;
    lambda_arg_file = file;
    process_http_response2(net, lambda_write_to_file);

    kdebug("Done, %d Bytes written to disk\n", lambda_arg_offset);
}

static Int lambda_state_block = 0;
static Int lambda_state_index = 0;
static Int lambda_state_written = 0;

void lambda_write_to_disk(Byte byte) {
    disk_drive_get_buffer(state.diskDrive)[lambda_state_index] = byte;
    lambda_state_index++;
    lambda_state_written++;

    if (lambda_state_index == DISK_DRIVE_BUFFER_SIZE) {
        lambda_state_index = 0;
        disk_drive_set_current_sector(state.diskDrive, lambda_state_block++);
        disk_drive_signal(state.diskDrive, DISK_DRIVE_SIGNAL_WRITE);
        motherboard_sleep((Byte) disk_drive_get_access_time(state.diskDrive));
    }
}

void f_update(ArgType file) {

    NetworkCard *net = state.networkCard;
    if (net == NULL) {
        kdebug("No network card found\n");
        return;
    }

    while (monitor_has_key_events(state.monitor)) {
        monitor_get_last_key_event(state.monitor);
    }
    kdebug("Please change disk and press any key.\n");
    while (!monitor_has_key_events(state.monitor)) {
        motherboard_sleep(1);
    }
    monitor_get_last_key_event(state.monitor);

    if (!disk_drive_has_disk(state.diskDrive)) {
        kdebug("Error no disk found\n");
        return;
    }

    // Init connection
    kdebug("Connecting to raw.githubusercontent.com...\n");
    network_set_target_ip(net, "raw.githubusercontent.com");
    network_set_target_port(net, 443);
    network_signal(net, NETWORK_SIGNAL_OPEN_SSL_TCP_CONNECTION);

    if (!network_is_connection_open(net)) {
        int errcode = network_get_connection_error(net);
        kdebug("Error connecting to github, error code: %d\n", errcode);
        return;
    }
    kdebug("Connected\n");

    // HTTP layer
    String *getPre = "GET /Magneticraft-Team/Magneticraft/1.12/src/main/resources/assets/magneticraft/cpu/";
    String *getPost = " HTTP/1.1\r\n"
            "Host: raw.githubusercontent.com\r\n"
            "Connection: close\r\n"
            "\r\n";

    // Send
    network_send(net, getPre, strlen(getPre));
    network_send(net, file, strlen(file));
    network_send(net, getPost, strlen(getPost));

    lambda_state_index = 0;
    lambda_state_block = 0;
    lambda_state_written = 0;
    process_http_response2(net, lambda_write_to_disk);

    // Write last block if needed
    if (lambda_state_index != 0) {
        disk_drive_set_current_sector(state.diskDrive, lambda_state_block++);
        disk_drive_signal(state.diskDrive, DISK_DRIVE_SIGNAL_WRITE);
    }
    kdebug("Done, %d Bytes written to disk\n", lambda_state_written);
}

void f_quarry(ArgType file) {
    if (state.robot == NULL) {
        kdebug("This computer doesn't support mining\n");
        return;
    }

    MiningRobot *rb = state.robot;
    Int size;
    if (!parsetInt(file, &size)) {
        kdebug("Invalid size: '%s'\n", file);
        return;
    }
    quarry_start(rb, size);
}