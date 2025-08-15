#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 12346
#define IP "192.168.1.165"

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
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "for /f \"tokens=5\" %%a in ('netstat -ano ^| findstr :%d ^| findstr LISTENING') do taskkill /PID %%a /F", PORT);
    system(cmd);
    Sleep(1000);
}

int kernel_run_process(const char *process_path)
{

    // Todo: Add Task table support for windows spawned processes.

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    if (!CreateProcess(NULL, (LPSTR)process_path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        fprintf(stderr, "CreateProcess failed (%lu)\n", GetLastError());
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

int monitor_running_tasks(char *outbuf, size_t outbuf_size)
{
    FILE *fp = _popen("tasklist", "r");
    if (!fp)
    {
        snprintf(outbuf, outbuf_size, "Error: could not run tasklist\n");
        return -1;
    }
    size_t used = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        if (used + strlen(line) < outbuf_size)
        {
            strcat(outbuf, line);
            used += strlen(line);
        }
        else
        {
            break;
        }
    }
    _pclose(fp);
    return 0;
}

// Kernel socket, listens and accepts IPC commands.
int main(void)
{

    cleanup_socket();

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    int sockfd, clientfd;
    struct sockaddr_in addr;
    char buf[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
        exit(1);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        fprintf(stderr, "setsockopt failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(IP);
    addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        exit(1);
    }

    if (listen(sockfd, 5) == SOCKET_ERROR)
    {
        fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        exit(1);
    }

    while (1)
    {
        clientfd = accept(sockfd, NULL, NULL);
        if (clientfd == INVALID_SOCKET)
        {
            fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
            continue;
        }

        // Reads and tokenizes arguments in the libclient command.
        ssize_t n = recv(clientfd, buf, sizeof(buf) - 1, 0);
        if (n > 0)
        {
            buf[n] = '\0';
            printf("Received command: %s\n", buf);

            char *cmd = strtok(buf, " ");
            char *arg = strtok(NULL, " ");
            if (cmd && strcmp(cmd, "run") == 0 && arg)
            {
                kernel_run_process(arg);
                send(clientfd, "OK\n", 3, 0);
            }
            else if (cmd && strcmp(cmd, "processes") == 0)
            {
                char procbuf[4096];
                monitor_running_tasks(procbuf, sizeof(procbuf));
                send(clientfd, procbuf, strlen(procbuf), 0);
            }
            else
            {
                send(clientfd, "Unknown command\n", 16, 0);
            }
        }
        closesocket(clientfd);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}