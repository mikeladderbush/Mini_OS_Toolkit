#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/kernel_daemon.sock"

int main(int argc, char *argv[])
{
    // Socket file descriptor, address struct and buffer for write.
    int sockfd;

    // sockaddr_un for IPC.
    struct sockaddr_un addr;
    char buf[256];

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        exit(1);
    }

    // Unix domain (local), Stream like TCP, default protocol.
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
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
        exit(1);
    }

    // Send command to kernel daemon.
    snprintf(buf, sizeof(buf), "%s", argv[1]);
    write(sockfd, buf, strlen(buf));

    close(sockfd);
    return 0;
}