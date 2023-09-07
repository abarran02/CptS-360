#include "commands.h"

char* parseLowestFile(char *pathname) {
    const char delim[2] = "/";
    char *token = strtok(pathname, delim);
    char *lowest;

    while (token != NULL) {
        lowest = token;
        token = strtok(NULL, delim);
    }

    return lowest;
}

void jumpToRoot(NODE *cwd) {
    while (cwd->parent != cwd) {
        cwd = cwd->parent;
    }
}

NODE* navigateToPath(NODE *cwd, char *pathname, int newNode) {
    const char delim[2] = "/";
    char *token = strtok(pathname, delim);
    NODE *currentChild;
    NODE *nwd = cwd; // start new working dir at cwd
    int found;

    if (pathname == NULL) {
        jumpToRoot(nwd);
        return nwd;
    }

    // if path is absolute
    if (pathname[0] == '/') {
        jumpToRoot(nwd);
        // next token, as token is empty to start
        token = strtok(NULL, delim);
    }

    // iterate over pathname
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            // points to cwd
            ;
        } else if (strcmp(token, "..") == 0) {
            // points to parent
            nwd = nwd->parent;
        } else {
            found = 0;
            // search all children for matching subdirectory
            currentChild = nwd->child;
            while (currentChild != NULL) {
                // match found, set cwd and break
                if (strcmp(token, currentChild->name) == 0) {
                    nwd = currentChild;
                    found = 1;
                    break;
                }

                // continue to sibling
                currentChild = currentChild->sibling;
            }

            if (!found) {
                if (newNode) {
                    token = strtok(NULL, delim);
                    if (token == NULL) {
                        return nwd;
                    }
                }

                return NULL;
            }
        }

        token = strtok(NULL, delim);
    }

    // should be new, but already exists
    if (newNode) {
        return NULL;
    }

    return nwd;
}

void rmHelper(NODE *filePtr) {
    // remove self from parent child list
    // case 1: filePtr is first child
    if (filePtr == filePtr->parent->child) {
        // fix linked list
        filePtr->parent->child = filePtr->sibling;
    } else {
        // case 2: filePtr in child list
        NODE *curChild = filePtr->parent->child;

        // find previous sibling to filePtr
        while (curChild->sibling != filePtr) {
            curChild = curChild->sibling;
        }

        curChild->sibling = filePtr->sibling;
    }

    free(filePtr);
}

int newFile(NODE *cwd, char *pathname, char type) {
    NODE *nwd = navigateToPath(cwd, pathname, 1);
    NODE *workingFile;
    char *filename;

    if (nwd == NULL) {
        return -1;  // already exists or target directory doesn't exist
    }

    if (nwd->child == NULL) {
        nwd->child = (NODE *)malloc(sizeof(NODE));
        nwd->child->parent = nwd;
        workingFile = nwd->child;  // reduce clutter for initialization
    } else {
        nwd = nwd->child;
        while (nwd->sibling != NULL) {
            nwd = nwd->sibling;
        }

        nwd->sibling = (NODE *)malloc(sizeof(NODE));
        nwd->sibling->parent = nwd->parent;
        workingFile = nwd->sibling;
    }

    filename = parseLowestFile(pathname);
    
    strcpy(workingFile->name, filename);
    workingFile->sibling = NULL;
    workingFile->child = NULL;
    workingFile->type = type;

    return 1;
}

int mkdir(NODE *cwd, char *pathname) {
    /* Make a new directory for a given pathname.
    Show error message (DIR pathname already exists!) if the directory already present in the
    filesystem. */
    return newFile(cwd, pathname, 'D');
}

int rmdir(NODE *cwd, char *pathname) {
    /* Remove the directory, if it is empty.
    Show error messages if:
    − The directory specified in pathname does not exist (DIR pathname does not exist!)
    − The directory is not empty (Cannot remove DIR pathname (not empty)!).
    */
    NODE *nwd = navigateToPath(cwd, pathname, 0);

    if (nwd == NULL) {
        return -1;  // directory not found
    } else if (nwd->child != NULL) {
        return -2;  // directory not empty
    }

    rmHelper(nwd);
}

int ls(NODE *cwd, char *pathname) {
    /* List the directory contents of pathname or CWD (if pathname not specified).
    Display an error message (No such file or directory: pathname) for an invalid pathname. */
    NODE *nwd = navigateToPath(cwd, pathname, 0);

    if (nwd == NULL) {
        return -1;
    }

    NODE *curChild = nwd->child;

    while (curChild != NULL) {
        printf("%c %s\n", curChild->type, curChild->name);
        curChild = curChild->sibling;
    }

    return 1;
}

int cd(NODE **cwd, char *pathname) {
    /* Change CWD to pathname, or to / if no pathname specified.
    Display an error message (No such file or directory: pathname) for an invalid pathname. */
    if (pathname == NULL) {
        while ((*cwd)->parent != *cwd) {
            *cwd = (*cwd)->parent;
        }
    }

    NODE *nwd = navigateToPath(*cwd, pathname, 0);

    if (nwd != NULL) {
        // path found, change cwd and return success
        *cwd = nwd;
        return 1;
    } else {
        // path not found
        return -1;
    }
}

int pwd(NODE *cwd) {
    /* Print the (absolute) pathname of CWD. */
    //char pathString[4096] = "";

    //pwdHelper(cwd, &pathString);

    char prependString[4096] = "";
    char pathString[4096] = "";

    while (!(cwd->parent == cwd)) {
        strcpy(prependString, cwd->name);
        strcat(prependString, "/");
        strcat(prependString, pathString);
        strcpy(pathString, prependString);

        cwd = cwd->parent;
    }

    printf("/%s\n", pathString);
}

int creat(NODE *cwd, char *pathname) {
    /* Create a new FILE node.
    Show error message (pathname already exists!) if the directory already present in the filesystem. */
    return newFile(cwd, pathname, 'F');
}

int rm(NODE *cwd, char *pathname) {
    /* Remove the FILE node specified by pathname.
    Display an error message (File pathname does not exist!) if there no such file exists.
    Display an error message (Cannot remove pathname (not a FILE)!) if pathname is not a FILE. */
    NODE *nwd = navigateToPath(cwd, pathname, 0);

    if (nwd == NULL) {
        return -1;  // file does not exist
    } else if (nwd->type != 'F') {
        return -2;  // is not a file
    }

    rmHelper(nwd);
}

int reload(NODE *root, char *filename) {
    /* Re-initalize the filesystem tree from the file filename. */
}

int save(NODE *root, char *filename) {
    /* Save the current filesystem tree in the file filename. */
}

int quit(NODE *root) {
    /* Save the filesystem tree in filename fssim_lastname.txt, then terminate the program.
    The "lastname" is your surname. */
    save(root, "fssim_barran.txt");
    exit(0);
}
