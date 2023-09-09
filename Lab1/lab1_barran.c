#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"

NODE *root;
NODE *cwd;
char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm",
                "reload", "save", "quit", 0};

int find_command(char *user_command) {
    int i = 0;
    while (cmd[i]) {
        if (strcmp(user_command, cmd[i]) == 0) {
            return i;
        }

        i++;
    }

    return -1; // command not found
}

int initialize() {
    root = (NODE *)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = NULL;
    root->child = NULL;
    root->type = 'D';
    cwd = root;
    printf("Filesystem initialized!\n");
}

int main() {
    char bashLine[4096], *command, *args;
    const char delim[2] = " ";
    int status;
    initialize();

    while(1) {
        printf("Enter command: ");
        fgets(bashLine, sizeof(bashLine), stdin); // get user input

        // tokenize input into command and arguments, then match command to function
        command = strtok(bashLine, delim);
        command[strcspn(command, "\n")] = 0; // remove newline char, https://stackoverflow.com/a/28462221
        args = strtok(NULL, delim);

        if (args != NULL) {
            args[strcspn(args, "\n")] = 0;
        }

        switch (find_command(command)) {
            case -1:
                printf("Unknown command: %s\n", command);
                break;
            case 0:
                status = mkdir(cwd, args);
                if (status == ALREADY_EXISTS) {
                    printf("Unable to create directory '%s': File exists\n", args);
                }
                break;
            case 1:
                status = rmdir(cwd, args);
                if (status == NOT_FOUND) {
                    printf("Unable to remove directory '%s': Does not exist\n", args);
                } else if (status == DIR_NOT_EMPTY) {
                    printf("Unable to remove directory '%s': Directory not empty\n", args);
                } else if (status == WRONG_TYPE) {
                    printf("Unable to remove directory '%s': Not a directory\n", args);
                }
                break;
            case 2:
                status = ls(cwd, args);
                if (status == NOT_FOUND) {
                    printf("Cannot access '%s': No such file or directory\n", args);
                }
                break;
            case 3:
                status = cd(&cwd, args);
                if (status == NOT_FOUND) {
                    printf("Cannot access '%s': Directory not found\n", args);
                }
                break;
            case 4:
                status = pwd(cwd);
                break;
            case 5:
                status = creat(cwd, args);
                if (status == ALREADY_EXISTS) {
                    printf("Unable to create file '%s': File exists\n", args);
                }
                break;
            case 6:
                status = rm(cwd, args);
                if (status == NOT_FOUND) {
                    printf("Unable to remove file '%s': Does not exist\n", args);
                } else if (status == WRONG_TYPE) {
                    printf("Unable to remove file '%s': Not a file\n", args);
                }
                break;
            case 7:
                status = reload(root, args);
                if (status == NOT_FOUND) {
                    printf("Unable to load file '%s': Does not exist", args);
                } else if (status == WRONG_TYPE) {
                    printf("Unable to load file '%s': Malformed data", args);
                }
                break;
            case 8:
                status = save(root, args);
                if (status == NOT_FOUND) {
                    printf("Unable to write to file '%s'", args);
                }
                break;
            case 9:
                quit(root);
                break;
        }
    }
}
