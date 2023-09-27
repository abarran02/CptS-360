#include "csapp.h"
#define N 2

int main()
{
    int status, i;
    pid_t pid;
    char *str = "cs360";

    /* Parent creates N children */
    for (i = 0; i < N; i++) {
        if ((pid = Fork()) == 0) {
            // cause seg fault by writing to read-only text segment
            str[2] = '4';
            exit(100+i);
        }
    }

    /* Parent reaps N children in no particular order */
    while ((pid = waitpid(-1, &status, 0)) > 0) {
        if (WIFEXITED(status)) {
            printf("child %d terminated normally with exit status=%d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            // terminated by a signal
            if (WTERMSIG(status) == SIGSEGV) {
                // terminated by a seg fault
                // print similar message to psignal()
                printf("child %d terminated by signal 11: Segmentation fault\n", pid);
            } else {
                // terminated by other signal
                printf("child %d terminated abnormally\n", pid);
            }
        } else {
            printf("child %d terminated abnormally\n", pid);
        }
    }

    /* The only normal termination is if there are no more children */
    if (errno != ECHILD)
	unix_error("waitpid error");

    exit(0);
}
