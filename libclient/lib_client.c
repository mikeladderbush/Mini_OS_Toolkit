#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define WINDOWS_IP "192.168.1.165"

int send_command_to_linux(const char *command, char *response, ssize_t response_size)
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

    ssize_t total = 0, n;
    while ((n = read(sockfd, response + total, response_size - total - 1)) > 0)
    {
        total += n;
        if (total >= response_size - 1)
            break;
    }
    response[total] = '\0';

    close(sockfd);
    return 0;
}

int send_command_to_windows(const char *command, char *response, ssize_t response_size)
{
    // Todo: create windows client socket that will send the mt_shell request to the windows kernel.
    int sockfd;
    struct sockaddr_in addr;
    char buf[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(WINDOWS_IP);
    addr.sin_port = htons(12346);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    snprintf(buf, sizeof(buf), "%s", command);
    if (write(sockfd, buf, strlen(buf)) < 0)
    {
        perror("write");
        close(sockfd);
        return -1;
    }

    ssize_t total = 0, n;
    while ((n = read(sockfd, response + total, response_size - total - 1)) > 0)
    {
        total += n;
        if (total >= response_size - 1)
            break;
    }
    response[total] = '\0';

    close(sockfd);
    return 0;
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