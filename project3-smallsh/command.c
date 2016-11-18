//
// Created by nsosnov on 11/15/16.
//

#include <malloc.h>

#include "command.h"

Command* new_command(size_t argCount)
{
    Command* ret = malloc(sizeof(Command));
    ret->redirectStdIn = false;
    ret->redirectStdOut = false;
    ret->background = false;
    ret->complete = false;

    if(argCount > 0)
    {
        ret->arguments = malloc(sizeof(char *) * argCount);
    }

    ret->numArgs = argCount;
    return ret;
}

void delete_command(Command* command)
{
    if(command == NULL)
    {
        return;
    }

    for(size_t i = 0; i < command->numArgs; i++)
    {
        free(command->arguments[i]);
    }

    if(command->numArgs > 0)
    {
        free(command->arguments);
    }

    if(command->command != NULL)
    {
        free(command->command);
    }

    free(command);

    command = NULL;
}


