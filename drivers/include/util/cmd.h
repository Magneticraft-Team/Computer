//
// Created by cout970 on 14/04/18.
//

#ifndef COMPUTER_CMD_H
#define COMPUTER_CMD_H

#include <fs/filesystem.h>

void cmd_ls(INodeRef folder);
INodeRef cmd_cd(INodeRef folder, String *name);

INodeRef cmd_mkdir(INodeRef folder, String *name);
INodeRef cmd_mkfile(INodeRef folder, String *name);
Boolean cmd_rm(INodeRef folder, String *name);

#endif //COMPUTER_CMD_H
