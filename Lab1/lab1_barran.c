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
	initialize();
	// other initialization as needed

	while(1) {
		printf("Enter command: ");
		// complete implementations
	}
}
