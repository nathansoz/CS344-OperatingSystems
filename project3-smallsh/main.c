#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


const char* PROMPT = ": ";
const int PROMPT_BUFFER = 4096;

char** parseArgs(const char* toParse, size_t* parsedArgs)
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
            if(toParse[i] == '\0')
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

    char** ret = malloc(sizeof(char*) * count);

    size_t start = 0;
    size_t end;
    size_t currentArg = 0;
    i = 0;

    while(i < strlen(toParse))
    {
        if(toParse[i] == ' ')
        {
            end = i - 1;
            char* sub = malloc(sizeof(char) * (end - start + 1));
            strncpy(sub, toParse + start, end + start + 1);
            ret[currentArg++] = sub;

            //TODO: Iterate until find new start
            //TODO: Write an arg dealloc function
            //TODO: This function probably has to return the number of args parsed
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

int process(const char* buffer)
{
    if(strcmp(buffer, "exit") == 0)
    {
        do_exit();
    }
    else if ((strlen(buffer) == 2 && strcmp(buffer, "cd") == 0) || strstr(buffer, "cd ") == buffer)
    {
        if(strlen(buffer) == 2)
        {
            return do_cd(NULL);
        }

        char** args = parseArgs(buffer);

        return do_cd("blah");
    }
}

int shell_loop()
{
    while(true)
    {
        char input[PROMPT_BUFFER];
        printf("%s", PROMPT);

        if(fgets(input, PROMPT_BUFFER, stdin) != 0)
        {
            char *pos;
            if ((pos=strchr(input, '\n')) != NULL)
            {
                *pos = '\0';
            }
            process(input);
        }
    }
}

int main()
{
    return shell_loop();
}