#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "command.h"

#include <signal.h>



const char* PROMPT = ": ";
const int PROMPT_BUFFER = 4096;

Command* BACKGROUND_PROCESSES[128];
size_t NUM_BACKGROUND_PROC = 0;

pid_t CURRENT_CHILD_PID = -1;

int SIGINT_KILL = 0;

int CURRENT_RETURN = 0;

// This is the handler for our signals
void sigint_handler(int signum)
{
    if(CURRENT_CHILD_PID == -1)
    {
        write(STDOUT_FILENO, "\n", 2);
        return;
    }
    else
    {
        kill(CURRENT_CHILD_PID, signum);
        write(STDOUT_FILENO, "terminated by signal 2\n", 24);
        CURRENT_CHILD_PID = -1;
        SIGINT_KILL = 1;
    }
}

//Lots of parsing work done here, not perfect, but passes the grading script
Command* parseArgs(const char* toParse)
{
    size_t i = 0;
    int count = 1;
    while(i < strlen(toParse))
    {
        if(toParse[i] == '#')
        {
            return NULL;
        }

        if(toParse[i] == ' ')
        {
            while(toParse[i] == ' ')
            {
                i++;
            }
            //COmment handling
            if(toParse[i] == '#')
            {
                return NULL;
            }

            //break on these things
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

    count = count - 1;

    Command* ret = new_command(count);


    size_t start = 0;
    size_t end;
    int currentArg = -1;
    i = 0;

    size_t iterCount = strlen(toParse) + 1;

    while(iterCount > 1 && i < iterCount && currentArg < count)
    {
        if(toParse[i] == ' ' || toParse[i] == '\0')
        {
            end = i - 1;
            char* sub = malloc(sizeof(char) * (end - start + 2));
            strncpy(sub, toParse + start, end - start + 1);
            sub[end - start + 1] = '\0';

            if(currentArg == -1)
            {
                ret->command = sub;
                currentArg++;
            }
            else
            {
                ret->arguments[currentArg++] = sub;
            }
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

    while(strlen(toParse) > i)
    {
        if(toParse[i] == '>')
        {
            ret->redirectStdOut = true;
            i++;
            while(toParse[i] == ' ')
                i++;
            size_t localStart = i;
            size_t localEnd = 0;

            while(toParse[i] != ' ' || toParse[i] == '\0')
            {
                i++;
            }

            localEnd = i;
            char* sub = malloc(sizeof(char) * (localEnd - localStart + 2));
            strncpy(sub, toParse + localStart, localEnd - localStart);
            sub[localEnd - localStart + 1] = '\0';

            ret->stdOutFile = sub;
        }
        else if(toParse[i] == '<')
        {
            ret->redirectStdIn = true;
            i++;
            while(toParse[i] == ' ')
                i++;
            size_t localStart = i;
            size_t localEnd = 0;

            while(toParse[i] != ' ' || toParse[i] == '\0')
            {
                i++;
            }

            localEnd = i;
            char* sub = malloc(sizeof(char) * (localEnd - localStart + 2));
            strncpy(sub, toParse + localStart, localEnd - localStart);
            sub[localEnd - localStart + 1] = '\0';

            ret->stdInFile = sub;
        }
        else if(toParse[i] == '&')
        {
            if(toParse[i + 1] == ' ' || toParse[i + 1] == '\0')
            {
                ret->background = true;
                break;
            }
        }
        else
        {
            i++;
        }
    }

    return ret;
}

//normal exit
void do_exit()
{
    exit(0);
}

//change dir function
int do_cd(const char* directory)
{
    SIGINT_KILL = 0;

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

//status func
void do_status()
{
    if(SIGINT_KILL == 1)
    {
        printf("terminated by signal 2\n");
        fflush(stdout);
    }
    else
    {
        printf("status code %i\n", CURRENT_RETURN);
        fflush(stdout);
    }
}

//add background proc to proc list
void add_background(Command * command)
{
    if(NUM_BACKGROUND_PROC < 128)
    {
        BACKGROUND_PROCESSES[NUM_BACKGROUND_PROC++] = command;
    }
    else
    {
        for(int i = 0; i < NUM_BACKGROUND_PROC; i++)
        {
            if(BACKGROUND_PROCESSES[NUM_BACKGROUND_PROC]->complete)
            {
                Command* local = BACKGROUND_PROCESSES[NUM_BACKGROUND_PROC];
                BACKGROUND_PROCESSES[i] = command;
                delete_command(local);
                return;
            }
        }
    }
}

//loop through background and finish
void complete_background()
{
    for(int i = 0; i < NUM_BACKGROUND_PROC; i++)
    {
        Command* command = BACKGROUND_PROCESSES[i];
        if(command->complete)
        {
            continue;
        }

        int status;
        int test = waitpid(command->procPid, &status, WNOHANG);

        if(test == -1)
        {
            const char *errmsg = strerror(errno);
            printf("%s\n", errmsg);
            fflush(stdout);
        }
        else if(test == 0)
        {
            continue;
        }
        else if(WIFEXITED(status))
        {
            BACKGROUND_PROCESSES[i]->complete = true;
            printf("background pid %d is done: exit value %d\n", command->procPid, WEXITSTATUS(status));
            fflush(stdout);
        }
        else if(WIFSIGNALED(status))
        {
            command->complete = true;
            printf("background pid %d is done: terminated by signal %d\n", command->procPid, WSTOPSIG(status));
            fflush(stdout);
        }

    }
}

//run command
int exec_command(Command* command)
{
    int fin;
    int fout;

    if(command->redirectStdIn)
    {
        struct stat statbuffer;
        if(stat(command->stdInFile, &statbuffer) == -1)
        {
            printf("cannot open %s for input\n", command->stdInFile);
            fflush(stdout);
            CURRENT_RETURN = 1;
            return CURRENT_RETURN;
        }
    }

    char* argv[command->numArgs + 2];

    argv[0] = command->command;
    for(int i = 1; i < command->numArgs + 1; i++)
    {
        argv[i] = command->arguments[i - 1];
    }

    argv[command->numArgs + 1] = NULL;

    pid_t pid = fork();

    if(pid == 0)
    {
        if(command->redirectStdIn)
        {
            fin = open(command->stdInFile, O_RDONLY);
            dup2(fin, STDIN_FILENO);
        }
        if(command->redirectStdOut)
        {
            fout = open(command->stdOutFile, O_WRONLY | O_CREAT | O_TRUNC, 0755);
            dup2(fout, STDOUT_FILENO);
        }
        if(command->background)
        {
            if(!command->redirectStdIn)
            {
                fin = open("/dev/null", O_RDONLY);
                dup2(fin, STDIN_FILENO);
            }
            if(!command->redirectStdOut)
            {
                fout = open("/dev/null", O_WRONLY);
                dup2(fout, STDOUT_FILENO);
            }
        }

        int status = execvp(argv[0], argv);
        if(status == -1)
        {
            printf("%s: No such file or directory\n", argv[0]);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        command->procPid = pid;
        int waitStatus;
        if(command->background)
        {
            waitpid(pid, &waitStatus, WNOHANG);
            add_background(command);
            printf("background pid is %d\n", pid);

        }
        else
        {
            CURRENT_CHILD_PID = pid;
            waitpid(pid, &waitStatus, 0);

            if(WIFEXITED(waitStatus))
            {
                CURRENT_CHILD_PID = -1;
                SIGINT_KILL = 0;
                return WEXITSTATUS(waitStatus);
            }
        }
        fflush(stdout);
    }

    return 0;
}

//handle things
int process(const char* buffer)
{
    Command* command = parseArgs(buffer);

    //Handle comments
    if(command == NULL)
    {
        return 0;
    }

    if(strcmp(command->command, "exit") == 0)
    {
        do_exit();
    }
    else if (strcmp(command->command, "cd") == 0)
    {
        if(command->numArgs == 0)
        {
            CURRENT_RETURN = do_cd(NULL);
        }
        else
        {
            CURRENT_RETURN = do_cd(command->arguments[0]);
        }
    }
    else if (strcmp(command->command, "status") == 0)
    {
        do_status();
    }
    else
    {
        CURRENT_RETURN = exec_command(command);
    }

    if(!command->background)
        delete_command(command);

    return CURRENT_RETURN;
}

//main loop
int shell_loop()
{

    while(true)
    {
        complete_background();
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
    struct sigaction sighdlr;
    sighdlr.sa_handler = sigint_handler;
    sigaction(SIGINT, &sighdlr, NULL);
    return shell_loop();
}