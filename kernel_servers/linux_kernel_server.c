#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>

#define PORT 12345

// Task structure.
typedef struct
{
    pid_t pid;
    char name[256];
    int status;
} Task;

// Task table for processes.
#define MAX_TASKS 32
Task task_table[MAX_TASKS];

void cleanup_socket()
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "sudo fuser -k %d/tcp", PORT);
    system(cmd);
    sleep(1);
}

int kernel_run_process(char process_path[])
{
    // Fork process for and add the child to the task table.
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

int monitor_running_tasks(char *outbuf, size_t outbuf_size)
{
    DIR *proc = opendir("/proc");
    struct dirent *entry;
    if (!proc)
    {
        snprintf(outbuf, outbuf_size, "Error: could not open /proc\n");
        return -1;
    }

    size_t used = 0;
    used += snprintf(outbuf + used, outbuf_size - used, "System Processes:\n");
    while ((entry = readdir(proc)) != NULL)
    {
        if (!isdigit(entry->d_name[0]))
        {
            continue;
        }
        char comm_path[256];
        snprintf(comm_path, sizeof(comm_path), "/proc/%.32s/comm", entry->d_name);

        FILE *comm = fopen(comm_path, "r");
        if (comm)
        {
            char name[256];
            if (fgets(name, sizeof(name), comm))
            {
                name[strcspn(name, "\n")] = 0;
                used += snprintf(outbuf + used, outbuf_size - used, "PID: %s, Name: %s\n", entry->d_name, name);
            }
            fclose(comm);
        }
        if (used >= outbuf_size - 64)
        {
            break;
        }
    }
    closedir(proc);
    return 0;
}

// Kernel socket, listens and accepts IPC commands.
int main(void)
{

    signal(SIGINT, cleanup_socket);
    signal(SIGTERM, cleanup_socket);
    signal(SIGHUP, cleanup_socket);

    cleanup_socket();

    int sockfd, clientfd;
    struct sockaddr_in addr;
    char buf[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(sockfd);
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
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

        // Reads and tokenizes arguments in the libclient command.
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
            else if (cmd && strcmp(cmd, "processes") == 0)
            {
                char procbuf[4096];
                monitor_running_tasks(procbuf, sizeof(procbuf));
                write(clientfd, procbuf, strlen(procbuf));
            }
            else
            {
                write(clientfd, "Unknown command\n", 16);
            }
        }
        close(clientfd);
    }

    close(sockfd);
    return 0;
}