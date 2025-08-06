#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    // Launch mt_shell_core in a new xterm window
    execlp("xterm", "xterm", "-geometry", "80x24", "-e", "./mt_shell_core", (char *)NULL);
    perror("execlp failed");
    return 1;
}