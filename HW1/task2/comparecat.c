// you need to include -lrt flag (i.e., real-time libraries or librt)
// to compile this code

#include <fcntl.h>   /* for O_RDONLY */
#include <stdarg.h>  /* for va_list */
#include <stdint.h>  /* for uint64 definition */
#include <stdio.h>
#include <stdlib.h>  /* for exit() */
#include <time.h>    /* for clock_gettime */
#include <unistd.h>  /* for STDOUT_FILENO */
#include "cat.c"

#define BILLION 1000000000L

int cathw(int argc, char *argv[]);
static void error(char *fmt, ...);

int main(int argc, char *argv[]) {
    uint64_t runtimeCathw, runtimeCat, diffTotal;
    struct timespec start, end;
    int i, bufferSize;

    // measure time for cathw
    clock_gettime(CLOCK_MONOTONIC, &start);    // mark start time
    cathw(argc, argv);    // call cathw routine
    clock_gettime(CLOCK_MONOTONIC, &end);    // mark the end time

    // calcuate difference
    runtimeCathw = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("Run time of cathw = %llu nanoseconds\n", (long long unsigned int) runtimeCathw);

    // measure time for system cat
    clock_gettime(CLOCK_MONOTONIC, &start);    // mark start time
    cat(argc, argv);
    clock_gettime(CLOCK_MONOTONIC, &end);    // mark the end time

    // calcuate difference
    runtimeCat = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("Run time of system cat = %llu nanoseconds\n", (long long unsigned int) runtimeCat);

    // difference between cathw and cat
    diffTotal = runtimeCathw - runtimeCat;
    printf("cat.c runs %llu nanoseconds faster than cathw\n", (long long unsigned int) diffTotal);

    return 0;
}

int cathw(int argc, char *argv[]) {
    int openFile, bytesRead;
    char nextChar[1];

    // iterate over each file in argv
    for (int i = 1; i < argc; i++) {
        // open file for reading
        openFile = open(argv[i], O_RDONLY);

        // ensure file opens successfully
        if (openFile == -1) {
            error("Unable to open file ", argv[i]);
        }

        // read file contents and print to screen
        bytesRead = read(openFile, nextChar, 1);
        while (bytesRead != 0) {    // 0 indicates EOF
            write(STDOUT_FILENO, nextChar, 1);
            bytesRead = read(openFile, nextChar, 1);
        }

        // close file
        close(openFile);
    }

    return 0;
}

// no modification needed
void error(char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}
