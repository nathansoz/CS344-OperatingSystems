#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "command.h"

const char* PROMPT = ": ";
const int PROMPT_BUFFER = 4096;

int CURRENT_RETURN = 0;

Command* parseArgs(const char* toParse, size_t* parsedArgs)
{

    size_t i = 0;
    size_t count = 1;
    while(i < strlen(toParse))
    {
        if(toParse[i] == ' ')
        {
            while(toParse[i] == ' ')
            {
                i++;
            }
            if(toParse[i] == '\0' || toParse[i] == '>' || toParse[i] == '<' || toParse[i] == '&')
            {
                break;
            }
            else
            {
                count++;
            }
        }
        else
        {
            i++;
        }
    }

    Command* ret = new_command(count);


    size_t start = 0;
    size_t end;
    size_t currentArg = 0;
    i = 0;

    size_t iterCount = strlen(toParse) + 1;

    while(iterCount > 1 && i < iterCount)
    {
        if(toParse[i] == ' ' || toParse[i] == '\0')
        {
            end = i - 1;
            char* sub = malloc(sizeof(char) * (end - start + 2));
            strncpy(sub, toParse + start, end + start + 1);
            sub[end - start + 1] = '\0';
            ret[currentArg++] = sub;
            i++;

            //TODO: Iterate until find new start
            //TODO: Write an arg dealloc function
            //TODO: This function probably has to return the number of args parsed
            while(toParse[i] == ' ')
            {
                i++;
            }

            start = i;
        }
        else
        {
            i++;
        }
    }

    if(parsedArgs != NULL)
    {
        *parsedArgs = count;
    }
    return ret;
}

void do_exit()
{
    exit(0);
}

int do_cd(const char* directory)
{
    if(NULL == directory)
    {
        const char* homeDirectory = getenv("HOME");

        if(NULL != homeDirectory)
        {
            return chdir(homeDirectory);
        }
        else
        {
            return -1;
        }
    }

    return chdir(directory);
}

void do_status()
{
    printf("status code %i\n", CURRENT_RETURN);
    fflush(stdout);
}

int process(const char* buffer)
{
    size_t numArgs = 0;
    char** args = parseArgs(buffer, &numArgs);

    if(strcmp(args[0], "exit") == 0)
    {
        do_exit();
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        if(numArgs == 1)
        {
            CURRENT_RETURN = do_cd(NULL);
        }
        else
        {
            CURRENT_RETURN = do_cd(args[1]);
        }
    }
    else if (strcmp(args[0], "status") == 0)
    {
        do_status();
    }
}

int shell_loop()
{

    while(true)
    {
        char input[PROMPT_BUFFER];
        printf("%s", PROMPT);
        fflush(stdout);

        if(fgets(input, PROMPT_BUFFER, stdin) != 0)
        {
            char *pos;
            if ((pos=strchr(input, '\n')) != NULL)
            {
                *pos = '\0';
            }

            if(strlen(input) > 0)
            {
                process(input);
            }
        }
    }
}

int main()
{
    return shell_loop();
}