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

// stat check for kernel daemon.
int is_linux_running()
{
    int sockfd, clientfd;
    struct sockaddr_in addr;
    char buf[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
}

int is_windows_running()
{
    int sockfd;
    struct sockaddr_in addr;
    int result = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return 0;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12346);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
    {
        result = 1;
    }

    close(sockfd);
    return result;
}

// Starts the linux kernel daemon.
void start_linux()
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

// Starts the windows kernel daemon.
void start_windows()
{
    // CreateProcess using windows kernel bin.
}

#define USAGE                                                                            \
    "usage:\n"                                                                           \
    " Minitool [options]\n"                                                              \
    "options:\n"                                                                         \
    " -e Environment to send command to 'both', 'linux', or 'windows', defaults to both" \
    " -c Command to be processed\n"                                                      \
    " -p Process for command to be enacted on\n"                                         \
    " -h Display help message\n"

/* OPTIONS DESCRIPTOR ==============================================*/

static struct option gLongOptions[] = {
    {"environment", required_argument, NULL, 'e'},
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
    char environment[256] = {0};
    char response[256] = {0};

    if (!is_linux_running())
    {
        start_linux();
    }

    if (!is_windows_running())
    {
        start_windows();
    }

    while ((option_char = getopt_long(argc, argv, "e:c:p:h", gLongOptions, NULL)) != -1)
    {
        switch (option_char)
        {
        case 'e': // Specified environment to receive command
            strncpy(environment, optarg, sizeof(environment) - 1);
            break;
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

    // Parses commands and builds full message for the kernel daemons.
    if (command[0] != '\0')
    {
        char full_command[512];
        if (process_path[0] != '\0')
            snprintf(full_command, sizeof(full_command), "%s %s", command, process_path);
        else
            snprintf(full_command, sizeof(full_command), "%s", command);

        if (send_command_to_daemons(full_command, response, sizeof(response), environment) == 0)
        {
            printf("%s\n", response);
        }
        else
        {
            printf("Failed to send command to daemons.\n");
        }
        return 0;
    }

    // Interactive mode
    printf("Minitool Shell (type 'exit' to quit)\n");
    while (1)
    {
        printf("> ");
        fflush(stdout);
        if (!fgets(command, sizeof(command), stdin))
            break;
        command[strcspn(command, "\n")] = 0;
        if (strcmp(command, "exit") == 0)
            break;

        if (send_command_to_daemons(command, response, sizeof(response), environment) == 0)
        {
            printf("%s\n", response);
        }
        else
        {
            printf("Failed to send command to daemons.\n");
        }

        return 0;
    }
}