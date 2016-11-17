//
// Created by nsosnov on 11/14/16.
//

#ifndef SMALLSH_COMMAND_H
#define SMALLSH_COMMAND_H

#include <stdbool.h>

typedef struct
{
    char* command;
    char** arguments;
    size_t numArgs;
    int return_code;
    bool redirectStdIn;
    char* stdInFile;
    bool redirectStdOut;
    char* stdOutFile;
    bool background;
} Command;

Command* new_command(size_t argCount);

void delete_command(Command* command);

#endif //SMALLSH_COMMAND_H
