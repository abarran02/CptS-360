#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
	char name[64];
	char type;
	struct node *child, *sibling, *parent;
} NODE;

enum State {
	SUCCESS,
	NOT_FOUND,
	ALREADY_EXISTS,
	DIR_NOT_EMPTY,
	WRONG_TYPE
};

typedef struct navState {
	NODE *nwd;
	enum State status;
} NavState;

char* parseLowestFile(char *pathname);
NODE* jumpToRoot(NODE *cwd);
NavState navigateToPath(NODE *cwd, char *pathname, int newNode);
void rmHelper(NODE *filePtr);
int newFile(NODE *cwd, char *pathname, char type);

int mkdir(NODE *cwd, char *pathname);
int rmdir(NODE *cwd, char *pathname);
int ls(NODE *cwd, char* pathname);
int cd(NODE **cwd, char *pathname);
int pwd(NODE *cwd);
int creat(NODE *cwd, char *pathname);
int rm(NODE *cwd, char *pathname);
int reload(NODE *root, char *filename);
int save(NODE *root, char *filename);
int quit(NODE *root);
