#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include "../libclient/lib_client.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int is_daemon_running()
{
    struct stat st;
    return (stat("/tmp/kernel_daemon.sock", &st) == 0);
}

void start_kernel_daemon()
{
    pid_t pid = fork();
    if (pid == 0)
    {
        execl("./kernel_server_bin", "./kernel_server_bin", (char *)NULL);
        perror("Failed to start kernel_server");
        exit(1);
    }
    usleep(200000);
}

#define USAGE                                    \
    "usage:\n"                                   \
    " Minitool [options]\n"                      \
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
    char response[256] = {0};

    if (!is_daemon_running())
    {
        start_kernel_daemon();
    }

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

    if (command[0] != '\0')
    {
        char full_command[512];
        if (process_path[0] != '\0')
        {
            snprintf(full_command, sizeof(full_command), "%s %s", command, process_path);
        }
        else
        {
            snprintf(full_command, sizeof(full_command), "%s", command);
        }

        if (send_command_to_daemon(full_command, response, sizeof(response)) == 0)
        {
            printf("Daemon: %s\n", response);
        }
        else
        {
            printf("Failed to send command to daemon.\n");
        }
        return 0;
    }

    // Interactive mode.
    printf("Minitool Shell (type 'exit' to quit)\n");
    while (1)
    {
        printf("> ");
        fflush(stdout);
        if (!fgets(command, sizeof(command), stdin))
        {
            break;
        }

        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "exit") == 0)
        {
            break;
        }

        if (send_command_to_daemon(command, response, sizeof(response)) == 0)
        {
            printf("Daemon: %s\n", response);
        }
        else
        {
            printf("Failed to send command to daemon.\n");
        }
    }

    return 0;
}