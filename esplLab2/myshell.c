#include <stdio.h>
#include "LineParser.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

int toWait = 1;

void handleProcessCommand(cmdLine *pCmdLine)
{
    if (pCmdLine->argCount < 2)
    {
        fprintf(stderr, "Error: Process ID missing\n");
        return;
    }

    pid_t pid = atoi(pCmdLine->arguments[1]);

    if (strcmp(pCmdLine->arguments[0], "stop") == 0)
    {
        if (kill(pid, SIGSTOP) == -1)
            perror("Error stopping process");
        else
            printf("Process %d stopped\n", pid);
    }
    else if (strcmp(pCmdLine->arguments[0], "wake") == 0)
    {
        if (kill(pid, SIGCONT) == -1)
            perror("Error waking process");
        else
            printf("Process %d continued\n", pid);
    }
    else if (strcmp(pCmdLine->arguments[0], "term") == 0)
    {
        if (kill(pid, SIGINT) == -1)
            perror("Error terminating process");
        else
            printf("Process %d terminated\n", pid);
    }
}

void protectSpacesInQuotes(char *s) {
    int inQuote = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == '"')
            inQuote = !inQuote;
        else if (s[i] == ' ' && inQuote)
            s[i] = 7;   // ASCII BEL as placeholder
    }
}

void restoreSpaces(cmdLine *cmd) {
    for (int i = 0; i < cmd->argCount; i++) {
        char *a = cmd->arguments[i];
        for (int j = 0; a[j]; j++)
            if (a[j] == 7)
                a[j] = ' ';
    }
}

int main(int argc, char **argv)
{
    int debug = 0;

    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        debug = 1;
    }

    while (1)
    {
        char dir[2048];
        getcwd(dir, sizeof(dir));
        printf("%s > ", dir);

        char input[2048];
        fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "quit") == 0)
        {
            printf("Exiting shell...\n");
            break;
        }

        if (input[strlen(input) - 1] == '&')
        {
            toWait = 0;
            input[strlen(input) - 1] = '\0';

            // Remove trailing space if present
            if (strlen(input) > 0 && input[strlen(input) - 1] == ' ')
                input[strlen(input) - 1] = '\0';
        }
        else
        {
            toWait = 1;
        }

        protectSpacesInQuotes(input);
        cmdLine *pCmdLine = parseCmdLines(input);
        restoreSpaces(pCmdLine);

        if (strcmp(pCmdLine->arguments[0], "cd") == 0)
        {
            if (pCmdLine->argCount < 2)
                fprintf(stderr, "cd: missing argument\n");
            else if (chdir(pCmdLine->arguments[1]) != 0)
                perror("cd failed");
            freeCmdLines(pCmdLine);
            continue;
        }

        /* TASK 3: STOP/WAKE/TERM */
        if (strcmp(pCmdLine->arguments[0], "stop") == 0 ||
            strcmp(pCmdLine->arguments[0], "wake") == 0 ||
            strcmp(pCmdLine->arguments[0], "term") == 0)
        {

            handleProcessCommand(pCmdLine);
            freeCmdLines(pCmdLine);
            continue;
        }

        /* fork and run normal commands */
        int pid = fork();

        if (pid == 0)
        {

            /* Task 2 - Output redirect */
            if (pCmdLine->outputRedirect != NULL)
            {
                int fd = open(pCmdLine->outputRedirect,
                              O_WRONLY | O_CREAT | O_TRUNC,
                              0666);
                if (fd == -1)
                    perror("failed opening file for output");
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            /* Task 2 - Input redirect */
            if (pCmdLine->inputRedirect != NULL)
            {
                int fd = open(pCmdLine->inputRedirect, O_RDONLY);
                if (fd == -1)
                    perror("failed opening file for input");
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            execvp(pCmdLine->arguments[0], pCmdLine->arguments);
            perror("execvp failed");
            exit(1);
        }
        else if (pid > 0)
        {

            if (debug)
            {
                fprintf(stderr, "Child executing: %s with argument count: %d\n",
                        pCmdLine->arguments[0], pCmdLine->argCount);
                fprintf(stderr, "PID for this process is %d\n", pid);
            }

            if (toWait)
                waitpid(pid, NULL, 0);
            else if (debug)
                printf("we will not wait until finishing the process\n");
        }
        else
        {
            perror("fork failed");
        }

        freeCmdLines(pCmdLine);
    }

    return 0;
}
