#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define STARTSIZEBUFFER 1024

void create_directory(char*, mode_t, int);
char** create_names(char*, int*);

/* This program create a new directory. If directory with same name
* already exist, an error will appear. if the -p opption was given
* error will not appear if direcotry with same name exist. Additionally
* by given the -p opption you can give to the program a path and the
* program will create all the directories in this path. for example:
* "./mkdir -p ex1/ex2/ex3" will create in the current directory ex1
* in ex1 create ex2 and in ex2 create ex3.
*/
int main(int argc, char * argv[]) {
    int checkP = 0;
    char* filename;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            checkP = 1;
        } else {
            filename = argv[i];
        }
    }

    mode_t md = 0775;
    create_directory(filename, md, checkP);

    return 0;
}

void create_directory(char* filename, mode_t md, int checkP) {
    if (checkP) {
        int size = 1;
        char ** names = create_names(filename, &size);
        for (int i = 0; i < size; i++) {
            if (mkdir(names[i], md) == -1) {
                if (errno != EEXIST) {
                    perror("error in create directory");
                    exit(EXIT_FAILURE);
                }
            }
            if (chdir(names[i]) == -1) {
                perror("error in change directory");
                exit(EXIT_FAILURE);
            }
        }
        return;
    }
    if (mkdir(filename, md) == -1) {
        perror("error in create directory");
        exit(EXIT_FAILURE);
    }
    
}

char** create_names(char* path, int* ptr) {
    size_t siz = 1;
    char ** names = (char**)calloc(siz * STARTSIZEBUFFER, (sizeof(char*)));
    if (names == NULL) {
        perror("error in allocate memory");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    char* token = strtok(path, "/");
    while (token != NULL) {
        size_t lengthStr = strlen(token) + 1;
        names[i] = (char*)malloc(lengthStr);
        if (names[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(names[j]);
            }
            free(names);
            perror("error int malloc function");
            exit(EXIT_FAILURE);
        }
        strncpy(names[i], token, lengthStr);
        names[i][lengthStr - 1] = '\0';
        token = strtok(NULL, "/");

        i++;
        if (i > siz * STARTSIZEBUFFER) {
            siz++;
            names = (char**)realloc(names, siz * STARTSIZEBUFFER);
            if (names == NULL) {
                perror("error in realloc memory");
                exit(EXIT_FAILURE);
            }
        }
    }

    *ptr = i;
    return names;
}