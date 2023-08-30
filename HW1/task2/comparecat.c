// you need to include -lrt flag (i.e., real-time libraries or librt)
// to compile this code

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>    /* for uint64 definition */
#include <string.h>
#include <time.h>    /* for clock_gettime */
#include <unistd.h>

#define BILLION 1000000000L
#define MAXLINE 4096    // maximum line length for bash

int cathw(int argc, char *argv[]);
static void error(char *fmt, ...);

int main(int argc, char *argv[]) {
    uint64_t diffCathw, diffCat, diffTotal;
    struct timespec start, end;
    char buffer[MAXLINE] = "cat";
    int i, bufferSize;

    // measure time for cathw
    clock_gettime(CLOCK_MONOTONIC, &start);    // mark start time
    cathw(argc, argv);    // call cathw routine
    clock_gettime(CLOCK_MONOTONIC, &end);    // mark the end time

    // calcuate difference
    diffCathw = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("Run time of cathw = %llu nanoseconds\n", (long long unsigned int) diffCathw);

    // build string for cat call
    for (i = 1; i < argc; i++) {
        strcat(buffer, " ");
        strcat(buffer, argv[i]);
    }
    // measure time for system cat
    clock_gettime(CLOCK_MONOTONIC, &start);    // mark start time
    system(buffer);    // call cat routine
    clock_gettime(CLOCK_MONOTONIC, &end);    // mark the end time

    // calcuate difference
    diffCat = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("Run time of system cat = %llu nanoseconds\n", (long long unsigned int) diffCat);

    // difference between cathw and cat
    diffTotal = diffCat - diffCathw;
    printf("cathw runs %llu nanoseconds faster than system cat\n", (long long unsigned int) diffTotal);

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
