//
// Created by nsosnov on 11/15/16.
//

#include <malloc.h>

#include "command.h"

Command* new_command(size_t argCount)
{
    Command* ret = malloc(sizeof(Command));
    ret->command = malloc(sizeof(char) * 4096);
    ret->arguments = malloc(sizeof(char*) * argCount);
    return ret;
}
