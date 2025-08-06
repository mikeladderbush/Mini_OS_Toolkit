#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/kernel_daemon.sock"

typedef struct
{
    pid_t pid;
    char name[256];
    int status;
} Task;

#define MAX_TASKS 32
Task task_table[MAX_TASKS];

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

int main(void)
{

    int sockfd, clientfd;
    struct sockaddr_un addr;
    char buf[256];

    unlink(SOCKET_PATH);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    if (listen(sockfd, 5) < 0)
    {
        perror("listen");
        close(sockfd);
        exit(1);
    }

    while (1)
    {
        clientfd = accept(sockfd, NULL, NULL);
        if (clientfd < 0)
        {
            perror("accept");
            continue;
        }

        ssize_t n = read(clientfd, buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = '\0';
            printf("Received command: %s\n", buf);

            char *cmd = strtok(buf, " ");
            char *arg = strtok(NULL, " ");
            if (cmd && strcmp(cmd, "run") == 0 && arg)
            {
                kernel_run_process(arg);
                write(clientfd, "OK\n", 3);
            }
            else
            {
                write(clientfd, "Unknown command\n", 16);
            }
        }
        close(clientfd);
    }

    close(sockfd);
    unlink(SOCKET_PATH);
    return 0;
}