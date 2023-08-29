// you need to include -lrt flag (i.e., real-time libraries or librt)
// to compile this code

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>	/* for uint64 definition */
#include <time.h>	/* for clock_gettime */
#include "cathw.c"

#define BILLION 1000000000L

main_cathw(int argc, char *argv[]) {
	// your implementation for cat using system calls
}

int main(int argc, char **argv) {
	uint64_t diff;
	struct timespec start, end;
	int i;

  	// measure time
	clock_gettime(CLOCK_MONOTONIC, &start);	// mark start time
	main_cathw(argc, argv);	// call cat routine
	clock_gettime(CLOCK_MONOTONIC, &end);	// mark the end time

  	// calcuate difference
	diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	printf("Run time of cathw = %llu nanoseconds\n", (long long unsigned int) diff);

	return 0;
}