#include "userapp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_CMD_LEN 256


// Register process with kernel module
void register_process(unsigned int pid) {
    char command[MAX_CMD_LEN] = "";
    snprintf(command, MAX_CMD_LEN, "echo \"%d\" > /proc/kmlab/status", pid);

    system(command);
}

int main(int argc, char* argv[]) {
    int __expire = 10;
    time_t start_time = time(NULL);

    if (argc == 2) {
        __expire = atoi(argv[1]);
    }

    register_process(getpid());

    // Terminate user application if the time is expired
    while (1) {
        if ((int)(time(NULL) - start_time) > __expire) {
            break;
        }
    }

	return 0;
}
