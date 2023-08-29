#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void filecopy(int ifd, int ofd);
static void error(char *fmt, ...); // can be used to generate error message

int main(int argc, char *argv[]) {
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
        while (bytesRead != 0) { // 0 indicates EOF
            write(STDOUT_FILENO, nextChar, 1);
            bytesRead = read(openFile, nextChar, 1);
        }

        // close file
        close(openFile);
    }

    exit(0);
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