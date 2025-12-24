#include <stdio.h>
#include "LineParser.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/wait.h>

#define HISTLEN 10

char* history[HISTLEN];
int hist_start = 0;   // index of oldest command
int hist_count = 0;   // number of stored commands

#define TERMINATED  -1
#define RUNNING      1
#define SUSPENDED    0

typedef struct process {
    cmdLine* cmd;
    pid_t pid;
    int status;
    struct process* next;
} process;

process* process_list = NULL;

void addToHistory(const char* line) {
    if (hist_count < HISTLEN) {
        history[(hist_start + hist_count) % HISTLEN] = strdup(line);
        hist_count++;
    } else {
        // overwrite oldest
        free(history[hist_start]);
        history[hist_start] = strdup(line);
        hist_start = (hist_start + 1) % HISTLEN;
    }
}

void printHistory() {
    for (int i = 0; i < hist_count; i++) {
        int idx = (hist_start + i) % HISTLEN;
        printf("%d %s\n", i, history[idx]);
    }
}

char* getHistory(int index) {
    if (index < 0 || index >= hist_count)
        return NULL;
    return history[(hist_start + index) % HISTLEN];
}

process* findProcess(process *list, pid_t pid)
{
    while (list)
    {
        if (list->pid == pid)
            return list;
        list = list->next;
    }
    return NULL;
}

cmdLine *copyCmdLine(const cmdLine *pCmdLine)
{
    if (!pCmdLine)
        return NULL;

    cmdLine *copy = malloc(sizeof(cmdLine));
    if (!copy)
        return NULL;

    copy->argCount = pCmdLine->argCount;
    copy->blocking = pCmdLine->blocking;
    copy->idx = pCmdLine->idx;
    copy->next = NULL;

    for (int i = 0; i < pCmdLine->argCount; i++)
        copy->arguments[i] = strdup(pCmdLine->arguments[i]);

    copy->inputRedirect =
        pCmdLine->inputRedirect ? strdup(pCmdLine->inputRedirect) : NULL;

    copy->outputRedirect =
        pCmdLine->outputRedirect ? strdup(pCmdLine->outputRedirect) : NULL;

    return copy;
}


//add the front of the list
void addProcess(process** process_list, cmdLine* cmd, pid_t pid) {
    process* newProc = (process*)malloc(sizeof(process));
    newProc->cmd = copyCmdLine(cmd);
    newProc->pid = pid;
    newProc->status = RUNNING;
    newProc->next = *process_list;
    *process_list = newProc;
}

void updateProcessList(process **process_list) {
    if (process_list == NULL)
    {
        return;
    }
    process *p = *process_list;

    while (p) {
        int status;
        pid_t result = waitpid(p->pid, &status,
                                WNOHANG | WUNTRACED | WCONTINUED);

        if (result > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status))
                updateProcessStatus(*process_list, p->pid, TERMINATED);
            else if (WIFSTOPPED(status))
                updateProcessStatus(*process_list, p->pid, SUSPENDED);
            else if(WIFCONTINUED(status))
                updateProcessStatus(*process_list, p->pid, RUNNING);
        }
        p = p->next;
    }
}

void updateProcessStatus(process* process_list, int pid, int status){
    if(process_list == NULL)
        return;
    process* p = process_list;
    while (p) {
        if (p->pid == pid) {
            p->status = status;
            return;
        }
        p = p->next;
    }
}

void printProcessList(process **process_list)
{
    updateProcessList(process_list);

    printf("PID\t\tCOMMAND\t\tSTATUS\n");

    process *curr = *process_list;
    process *prev = NULL;

    while (curr)
    {
        printf("%d\t\t%s\t\t%s\n",
               curr->pid,
               curr->cmd->arguments[0],
               curr->status == RUNNING ? "Running" :
               curr->status == SUSPENDED ? "Suspended" : "Terminated");

        if (curr->status == TERMINATED)
        {
            process *toDelete = curr;

            if (prev)
                prev->next = curr->next;
            else
                *process_list = curr->next;

            curr = curr->next;

            freeCmdLines(toDelete->cmd);
            free(toDelete);
        }
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }
}

int toWait = 1;

void handleProcessCommand(cmdLine *pCmdLine)
{
    if (pCmdLine->argCount < 2)
    {
        fprintf(stderr, "Error: Process ID missing\n");
        return;
    }

    pid_t pid = atoi(pCmdLine->arguments[1]);

    // Find the process in your list
    process* p = findProcess(process_list, pid);
    if (!p) {
        fprintf(stderr, "Process %d not found in process list\n", pid);
        return;
    }

    if (strcmp(pCmdLine->arguments[0], "kuku") == 0) {
        if (kill(pid, SIGCONT) == -1)
            perror("Error waking process");
        else
            p->status = RUNNING;   // <- update status here
    }
    else if (strcmp(pCmdLine->arguments[0], "zzzz") == 0) {
        if (kill(pid, SIGSTOP) == -1)
            perror("Error stopping process");
        else
            p->status = SUSPENDED; // <- update status here
    }
    else if (strcmp(pCmdLine->arguments[0], "blast") == 0) {
        if (kill(pid, SIGINT) == -1)
            perror("Error terminating process");
        else
            p->status = TERMINATED; // <- update status here
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

void freeProcessList(process* process_list) {
    process* current = process_list;
    process* next;

    while (current != NULL) {
        next = current->next;
        freeCmdLines(current->cmd);
        free(current);
        current = next;
    }
    process_list = NULL;
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
        if (strlen(input) == 0)
            continue;

        if (strcmp(input, "history") == 0) {
            printHistory();
            continue;
        }

        if (strcmp(input, "!!") == 0) {
            if (hist_count == 0) {
                printf("No commands in history\n");
                continue;
            }
            strcpy(input, history[(hist_start + hist_count - 1) % HISTLEN]);
            printf("%s\n", input);
        }

        else if (input[0] == '!' && isdigit(input[1])) {
            int n = atoi(&input[1]);
            char* cmd = getHistory(n);
            if (!cmd) {
                printf("Invalid history index\n");
                continue;
            }
            strcpy(input, cmd);
            printf("%s\n", input);
        }

        addToHistory(input);


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

    
        if (strcmp(pCmdLine->arguments[0], "kuku") == 0 ||
            strcmp(pCmdLine->arguments[0], "zzzz") == 0 ||
            strcmp(pCmdLine->arguments[0], "blast") == 0)
        {

            handleProcessCommand(pCmdLine);
            freeCmdLines(pCmdLine);
            continue;
        }
        if (strcmp(pCmdLine->arguments[0], "procs") == 0) {
            printProcessList(&process_list);
            freeCmdLines(pCmdLine);
            continue;
        }

        if (pCmdLine->next != NULL) {
            int pipefd[2];

            /* Illegal redirections */
            if (pCmdLine->outputRedirect != NULL) {
                fprintf(stderr, "Error: output redirection on left side of pipe\n");
                freeCmdLines(pCmdLine);
                continue;
            }
            if (pCmdLine->next->inputRedirect != NULL) {
                fprintf(stderr, "Error: input redirection on right side of pipe\n");
                freeCmdLines(pCmdLine);
                continue;
            }

            pipe(pipefd);

            /* ---- First child (left command) ---- */
            pid_t pid1 = fork();
            if (pid1 == 0) {
                /* stdout → pipe */
                dup2(pipefd[1], STDOUT_FILENO);

                close(pipefd[0]);
                close(pipefd[1]);

                /* input redirection allowed */
                if (pCmdLine->inputRedirect) {
                    int fd = open(pCmdLine->inputRedirect, O_RDONLY);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                execvp(pCmdLine->arguments[0], pCmdLine->arguments);
                perror("execvp failed");
                exit(1);
            }

            /* ---- Second child (right command) ---- */
            pid_t pid2 = fork();
            if (pid2 == 0) {
                /* stdin ← pipe */
                dup2(pipefd[0], STDIN_FILENO);

                close(pipefd[1]);
                close(pipefd[0]);

                /* output redirection allowed */
                if (pCmdLine->next->outputRedirect) {
                    int fd = open(pCmdLine->next->outputRedirect,
                                O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                execvp(pCmdLine->next->arguments[0],
                    pCmdLine->next->arguments);
                perror("execvp failed");
                exit(1);
            }

            /* ---- Parent ---- */

            addProcess(&process_list, pCmdLine, pid1);
            addProcess(&process_list, pCmdLine->next, pid2);

            close(pipefd[0]);
            close(pipefd[1]);

            if (toWait) {
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
            }

            freeCmdLines(pCmdLine);
            continue;
        }
        
        else{ 
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

                addProcess(&process_list, pCmdLine, pid);
                
                if (toWait)
                {
                    int status;
                    waitpid(pid, &status, 0);

                    process *p = findProcess(process_list, pid);
                    if (p && (WIFEXITED(status) || WIFSIGNALED(status)))
                        p->status = TERMINATED;
                }

                else if (debug)
                    printf("we will not wait until finishing the process\n");
            }
            else
            {
                perror("fork failed");
            }
    }
        freeCmdLines(pCmdLine);
    }

    return 0;
}
