#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/kernel_daemon.sock"

int send_command_to_daemon(const char *command, char *response, size_t response_size)
{
    // Socket file descriptor, address struct and buffer for write.
    int sockfd;

    // sockaddr_un for IPC.
    struct sockaddr_un addr;
    char buf[256];

    // Unix domain (local), Stream like TCP, default protocol.
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    // Blank address pointer.
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    // Put path into UNIX path.
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

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