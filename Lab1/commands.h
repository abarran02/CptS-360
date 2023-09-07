#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
	char name[64];
	char type;
	struct node *child, *sibling, *parent;
} NODE;

void cdToRoot(NODE *cwd);
NODE* navigateToPath(NODE *cwd, char *pathname);

int mkdir(NODE *cwd, char *pathname);
int rmdir(NODE *cwd, char *pathname);
int ls(NODE *cwd, char* pathname);
int cd(NODE *cwd, char *pathname);
int pwd(NODE *cwd);
int creat(NODE *cwd, char *pathname);
int rm(NODE *cwd, char *pathname);
int reload(NODE *root, char *filename);
int save(NODE *root, char *filename);
int quit(NODE *root);
