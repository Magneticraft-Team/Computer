//
// Created by cout970 on 14/04/18.
//

#include <fs/filesystem.h>
#include <debug.h>

void cmd_ls(INodeRef folder) {
    struct DirectoryIterator iter;
    struct INode node;

    fs_iterInit(folder, &iter);

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

Boolean cmd_rm(INodeRef folder, String *name) {
    INodeRef child = fs_findFile(folder, name);
    if (child == FS_NULL_INODE_REF) {
        kdebug("Error %s not found\n", name);
        return FALSE;
    }

    if (fs_delete(folder, child)) {
        kdebug("Error unable to remove file '%s'\n", name);
        return FALSE;
    }
    return TRUE;
}

INodeRef cmd_mkdir(INodeRef folder, String *name) {
    INodeRef res = fs_create(folder, name, FS_FLAG_DIRECTORY);
    if (res == FS_NULL_INODE_REF) {
        kdebug("Error unable to create '%s'\n", name);
    }
    return res;
}

INodeRef cmd_mkfile(INodeRef folder, String *name) {
    INodeRef res = fs_create(folder, name, FS_FLAG_FILE);
    if (res == FS_NULL_INODE_REF) {
        kdebug("Error unable to create '%s'\n", name);
    }
    return res;
}

INodeRef cmd_cd(INodeRef folder, String *name) {
    INodeRef child = fs_findFile(folder, name);

    if (child == FS_NULL_INODE_REF) {
        kdebug("Error %s doesn't exist\n", name);
        return folder;
    }

    struct INode node;
    fs_getINode(child, &node);

    if (node.flags != FS_FLAG_DIRECTORY) {
        kdebug("Error %s is not a folder\n", name);
        return folder;
    }
    return child;
}