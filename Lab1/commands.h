#include <stdio.h>
#include <stdlib.h>

typedef struct node {
	char name[64];
	char type;
	struct node *child, *sibling, *parent;
} NODE;

int mkdir(NODE *cwd, char *newDir);
int rmdir(NODE *cwd, char *rmTarget);
int ls(NODE *cwd);
int cd(NODE *cwd, char *newDir);
int pwd(NODE *cwd);
int creat(NODE *cwd, char *fileTarget);
int rm(NODE *cwd, char *rmTarget);
int reload(NODE *cwd, char *filename);
int save(NODE *cwd, char *filename);
int quit(NODE *cwd);
