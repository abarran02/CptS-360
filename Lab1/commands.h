#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 4096  // maximum bash line length

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
	WRONG_TYPE,
	NOT_ENOUGH_ARGS
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
void absoluteWd(NODE *cwd, char *pathString);
void saveRecursive(FILE *fp, NODE *nodePtr);

int mkdir(NODE *cwd, char *pathname);
int rmdir(NODE *cwd, char *pathname);
int ls(NODE *cwd, char* pathname);
int cd(NODE **cwd, char *pathname);
void pwd(NODE *cwd);
int creat(NODE *cwd, char *pathname);
int rm(NODE *cwd, char *pathname);
int reload(NODE *root, char *filename);
int save(NODE *root, char *filename);
void quit(NODE *root);
