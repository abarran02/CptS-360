#include "commands.h"

int mkdir(NODE *cwd, char *newDir) {
    /* Make a new directory for a given pathname.
    Show error message (DIR pathname already exists!) if the directory already present in the
    filesystem. */
}

int rmdir(NODE *cwd, char *rmTarget) {
    /* Remove the directory, if it is empty.
    Show error messages if:
    − The directory specified in pathname does not exist (DIR pathname does not exist!)
    − The directory is not empty (Cannot remove DIR pathname (not empty)!).
    */
}

int ls(NODE *cwd) {
    /* List the directory contents of pathname or CWD (if pathname not specified).
    Display an error message (No such file or directory: pathname) for an invalid pathname. */
}

int cd(NODE *cwd, char *newDir) {
    /* Change CWD to pathname, or to / if no pathname specified.
    Display an error message (No such file or directory: pathname) for an invalid pathname. */
}

int pwd(NODE *cwd) {
    /* Print the (absolute) pathname of CWD. */
}

int creat(NODE *cwd, char *fileTarget) {
    /* Create a new FILE node.
    Show error message (pathname already exists!) if the directory already present in the filesystem. */
}

int rm(NODE *cwd, char *rmTarget) {
    /* Remove the FILE node specified by pathname.
    Display an error message (File pathname does not exist!) if there no such file exists.
    Display an error message (Cannot remove pathname (not a FILE)!) if pathname is not a FILE. */
}

int reload(NODE *cwd, char *filename) {
    /* Re-initalize the filesystem tree from the file filename. */
}

int save(NODE *cwd, char *filename) {
    /* Save the current filesystem tree in the file filename. */
}

int quit(NODE *cwd) {
    /* Save the filesystem tree in filename fssim lastname.txt, then terminate the program.
    The "lastname" is your surname. */
}
