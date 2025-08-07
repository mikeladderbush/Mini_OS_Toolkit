#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#define SOCKET_PATH "/tmp/linux_kernel_daemon.sock"

int send_command_to_linux(const char *command, char *response, size_t response_size)
{
    // Socket file descriptor, address struct and buffer for write.
    int sockfd;

    // sockaddr_in for IPC.
    struct sockaddr_in addr;
    char buf[256];

    // Unix domain (local), Stream like TCP, default protocol.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    // Blank address pointer.
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(12345);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    // Send command to kernel daemon.
    snprintf(buf, sizeof(buf), "%s", command);
    if (write(sockfd, buf, strlen(buf)) < 0)
    {
        perror("write");
        close(sockfd);
        return -1;
    }

    if (response && response_size > 0)
    {
        ssize_t n = read(sockfd, response, response_size - 1);
        if (n > 0)
        {
            response[n] = '\0';
        }
        else
        {
            response[0] = '\0';
        }
    }

    close(sockfd);
    return 0;
}

int send_command_to_windows(const char *command, char *response, size_t response_size)
{
    // Todo: create windows client socket that will send the mt_shell request to the windows kernel.
    char buf[256];
    snprintf(buf, sizeof(buf), "%s", command);
    if (response && response_size > 0)
        response[0] = '\0';
    return -1;
}

int send_command_to_daemons(const char *command, char *response, size_t response_size, const char *environment)
{
    char linux_resp[4096] = "", win_resp[4096] = "";
    int ok = 0;

    if (environment[0] == '\0' || strcmp(environment, "both") == 0)
    {
        // Send to both
        if (send_command_to_linux(command, linux_resp, sizeof(linux_resp)) == 0)
            ok = 1;
        if (send_command_to_windows(command, win_resp, sizeof(win_resp)) == 0)
            ok = 1;
        snprintf(response, response_size, "--- Linux ---\n%s\n--- Windows ---\n%s\n", linux_resp, win_resp);
    }
    else if (strcmp(environment, "linux") == 0)
    {
        if (send_command_to_linux(command, linux_resp, sizeof(linux_resp)) == 0)
            ok = 1;
        snprintf(response, response_size, "--- Linux ---\n%s\n", linux_resp);
    }
    else if (strcmp(environment, "windows") == 0)
    {
        if (send_command_to_windows(command, win_resp, sizeof(win_resp)) == 0)
            ok = 1;
        snprintf(response, response_size, "--- Windows ---\n%s\n", win_resp);
    }
    else
    {
        snprintf(response, response_size, "Unknown environment: %s\n", environment);
    }
    return ok ? 0 : -1;
}