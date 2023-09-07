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

NODE* jumpToRoot(NODE *cwd) {
    while (cwd->parent != cwd) {
        cwd = cwd->parent;
    }
    return cwd;
}

NavState navigateToPath(NODE *cwd, char *pathname, int newNode) {
    const char delim[2] = "/";
    NODE *currentChild;
    NODE *nwd = cwd; // start new working dir at cwd
    int found;

    // don't want strtok to mutate pathname
    char pathnameCpy[MAX_LINE];
    strcpy(pathnameCpy, pathname);

    if (pathnameCpy == NULL) {
        nwd = jumpToRoot(nwd);
        return (NavState) { nwd, SUCCESS };
    }

    // if path is absolute
    if (pathnameCpy[0] == '/') {
        nwd = jumpToRoot(nwd);
    }

    char *token = strtok(pathnameCpy, delim);

    // iterate over pathnameCpy
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
                        return (NavState) { nwd, SUCCESS };
                    }
                }

                return (NavState) { NULL, NOT_FOUND };
            }
        }

        token = strtok(NULL, delim);
    }

    // should be new, but already exists
    if (newNode) {
        return (NavState) { NULL, ALREADY_EXISTS };
    }

    return (NavState) { nwd, SUCCESS };
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
    NavState navigate = navigateToPath(cwd, pathname, 1);

    if (navigate.status != SUCCESS) {
        return navigate.status;
    }

    NODE *nwd = navigate.nwd;
    NODE *workingFile;
    char *filename;

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

    return SUCCESS;
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
    NavState navigate = navigateToPath(cwd, pathname, 0);

    if (navigate.status != SUCCESS) {
        return navigate.status;
    }

    NODE *nwd = navigate.nwd;

    if (nwd->child != NULL) {
        return DIR_NOT_EMPTY;
    } else if (nwd->type != 'D') {
        return WRONG_TYPE;  // not a directory
    }

    rmHelper(nwd);
    return SUCCESS;
}

int ls(NODE *cwd, char *pathname) {
    /* List the directory contents of pathname or CWD (if pathname not specified).
    Display an error message (No such file or directory: pathname) for an invalid pathname. */
    NODE *curChild;

    if (pathname == NULL) {
        curChild = cwd->child;
    } else {
        NavState navigate = navigateToPath(cwd, pathname, 0);

        if (navigate.status != SUCCESS) {
            return navigate.status;
        }

        curChild = navigate.nwd->child;
    }

    while (curChild != NULL) {
        printf("%c %s\n", curChild->type, curChild->name);
        curChild = curChild->sibling;
    }

    return SUCCESS;
}

int cd(NODE **cwd, char *pathname) {
    /* Change CWD to pathname, or to / if no pathname specified.
    Display an error message (No such file or directory: pathname) for an invalid pathname. */
    NavState navigate = navigateToPath(*cwd, pathname, 0);

    if (navigate.status != SUCCESS) {
        return navigate.status;
    }

    *cwd = navigate.nwd;
    return SUCCESS;
}

void absoluteWd(NODE *cwd, char *pathString) {
    char prependString[MAX_LINE] = "";

    while (!(cwd->parent == cwd)) {
        strcpy(prependString, cwd->name);
        strcat(prependString, "/");
        strcat(prependString, pathString);
        strcpy(pathString, prependString);

        cwd = cwd->parent;
    }
}

int pwd(NODE *cwd) {
    /* Print the (absolute) pathname of CWD. */
    char pathString[MAX_LINE] = "";
    absoluteWd(cwd, pathString);
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
    NavState navigate = navigateToPath(cwd, pathname, 0);

    if (navigate.status != SUCCESS) {
        return navigate.status;
    }

    NODE *nwd = navigate.nwd;

    if (nwd->type != 'F') {
        return WRONG_TYPE;  // not a file
    }

    rmHelper(nwd);
    return SUCCESS;
}

int reload(NODE *root, char *filename) {
    /* Re-initalize the filesystem tree from the file filename. */
    FILE* file = fopen(filename, "r"); /* should check the result */
    char line[MAX_LINE], *token;
    const char delim[3] = " \n";

    while (fgets(line, sizeof(line), file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */

        token = strtok(line, delim);
        if (strcmp(token, "D") == 0) {
            token = strtok(NULL, delim);
            token[strcspn(token, "\n")] = 0;

            mkdir(root, token);
        } else if (strcmp(token, "F") == 0) {
            token = strtok(NULL, delim);
            token[strcspn(token, "\n")] = 0;

            creat(root, token);
        } else {
            return WRONG_TYPE;
        }
    }
}

void saveRecursive(FILE *fp, NODE *nodePtr) {
    NODE *curNode = nodePtr->child;
    char pathString[MAX_LINE] = "";
    absoluteWd(nodePtr, pathString);

    fprintf(fp, "%c /%s\n", nodePtr->type, pathString);

    while (curNode != NULL) {
        saveRecursive(fp, curNode);
        curNode = curNode->sibling;
    }
}

int save(NODE *root, char *filename) {
    /* Save the current filesystem tree in the file filename. */
    FILE *fp = fopen(filename, "w+"); // open a file stream
    if (fp == NULL) {
        return NOT_FOUND;
    }

    saveRecursive(fp, root);

    fclose(fp); // close file stream when done
}

void quit(NODE *root) {
    /* Save the filesystem tree in filename fssim_lastname.txt, then terminate the program.
    The "lastname" is your surname. */
    save(root, "fssim_barran.txt");
    exit(0);
}
