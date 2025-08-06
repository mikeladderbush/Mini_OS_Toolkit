#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/wait.h>

int kernel_run_process(char process_path[]);

typedef struct
{
    pid_t pid;
    char name[256];
    int status;
} Task;

#define MAX_TASKS 32
Task task_table[MAX_TASKS];

#define USAGE                                    \
    "usage:\n"                                   \
    " Kernel [options]\n"                        \
    "options:\n"                                 \
    " -c Command to be processed\n"              \
    " -p Process for command to be enacted on\n" \
    " -h Display help message\n"

/* OPTIONS DESCRIPTOR ==============================================*/

static struct option gLongOptions[] = {
    {"command", required_argument, NULL, 'c'},
    {"process", required_argument, NULL, 'p'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

/* Main ===========================================================*/
int main(int argc, char **argv)
{

    int option_char;
    char command[256] = {0};
    char process_path[256] = {0};

    while ((option_char = getopt_long(argc, argv, "c:p:h", gLongOptions, NULL)) != -1)
    {
        switch (option_char)
        {
        case 'c': // Command to enact
            strncpy(command, optarg, sizeof(command) - 1);
            break;
        case 'p': // Process to fork
            strncpy(process_path, optarg, sizeof(process_path) - 1);
            break;
        case 'h': // Help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        default:
            fprintf(stderr, "%s", USAGE);
            exit(1);
        }
    }

    if (command[0] == '\0')
    {
        fprintf(stderr, "Error: No command specified.\n");
        fprintf(stderr, "%s", USAGE);
        exit(1);
    }

    if (process_path[0] == '\0')
    {
        fprintf(stderr, "Error: No process path specified.\n");
        fprintf(stderr, "%s", USAGE);
        exit(1);
    }

    if (strcmp(command, "run") == 0)
    {
        kernel_run_process(process_path);
    }
    else
    {
        fprintf(stderr, "%s", USAGE);
        exit(1);
    }

    return 0;
}

int kernel_run_process(char process_path[])
{
    pid_t p = fork();
    if (p == 0)
    {
        execl(process_path, process_path, (char *)NULL);
        perror("execl failed");
        exit(1);
    }
    else if (p > 0)
    {
        int task_slot;
        for (task_slot = 0; task_slot < MAX_TASKS; task_slot++)
        {
            if (task_table[task_slot].pid == 0)
            {
                // Add new task to table.
                task_table[task_slot].pid = p;
                strncpy(task_table[task_slot].name, process_path, sizeof(task_table[task_slot].name) - 1);
                task_table[task_slot].name[sizeof(task_table[task_slot].name) - 1] = '\0';
                task_table[task_slot].status = 1; // 1 = running
                break;
            }
        }

        if (task_slot == MAX_TASKS)
        {
            fprintf(stderr, "Error: Task table full. \n");
            return -1;
        }
        waitpid(p, NULL, 0);
    }
    else
    {
        perror("fork failed");
        return -1;
    }

    return 0;
}