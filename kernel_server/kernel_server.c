#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#define USAGE
"usage:\n"
    " Kernel [options]\n"
    "options:\n"
    " -pid Process to be forked as a path\n"
    " -h Display help message\n"

    /* OPTIONS DESCRIPTOR ==============================================*/

    static struct option gLongOptions[] = {
        {"process", required_argument, NULL, 'pid'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}};

/* Main ===========================================================*/
int main(int argc, char **argv)
{

    int option_char;
    char process_path[256] = {0};

    while ((option_char = getopt_long(argc, argv, "p:h", gLongOptions, NULL)) != -1)
    {
        switch (option_char)
        {
        case 'p': // Process to fork
            strncpy(process_path, optarg, sizeof(process_path) - 1);
            break;
        case ' h': // Help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        default:
            fprintf(stderr, "%s", USAGE);
            exit(1);
        }
    }

    if (process_path[0] == '\0') {
        fprintf(stderr, "Error: No process path specified.\n");
        fprintf(stderr, "%s", USAGE);
        exit(1);
    }

    pid_t p = fork();
    if (p == 0)
    {
        execl(process_path, process_path, (char *)NULL);
        perror("execl failed");
        exit(1);
    }
    else if (p > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("fork failed");
        exit(1);
    }

    return 0;
}