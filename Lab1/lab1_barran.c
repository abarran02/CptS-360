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
    root->sibling = 0;
    root->child = 0;
    root->type = 'D';
    cwd = root;
    printf("Filesystem initialized!\n");
}

int main() {
    char bashLine[4096], *command, *args;
    const char delim[2] = " ";
    int status;
    initialize();
    // other initialization as needed

    while(1) {
        printf("Enter command: ");
        fgets(bashLine, sizeof(bashLine), stdin); // get user input

        // tokenize input into command and arguments, then match command to function
        command = strtok(bashLine, delim);
        command[strcspn(command, "\n")] = 0; // remove newline char, https://stackoverflow.com/a/28462221
        args = strtok(NULL, delim);

        switch (find_command(command)) {
            case -1:
                printf("Unknown command: %s\n", command);
                break;
            case 0:
                status = mkdir(cwd, args);
                break;
            case 1:
                status = rmdir(cwd, args);
                break;
            case 2:
                status = ls(cwd, args);
                break;
            case 3:
                if (args == NULL) {
                    status = cd(root, args);
                } else {
                    status = cd(cwd, args);
                }
                break;
            case 4:
                status = pwd(cwd);
                break;
            case 5:
                status = creat(cwd, args);
                break;
            case 6:
                status = rm(cwd, args);
                break;
            case 7:
                status = reload(cwd, args);
                break;
            case 8:
                status = save(cwd, args);
                break;
            case 9:
                status = quit(cwd);
                break;
        }
    }
}
