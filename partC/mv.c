#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

char* construct_path(char*,char*);

/* This program do two things depend on the input. The first argument
* must be a file name, and the second argument could be a new file name
* or a directory name. If the second argument is a new file name(the program
* not found directory with this name) its rename the name of the file that
* given in the first argument. If the program found directory with the name
* that given in the second argument she move the file to this directory.
*/
int main (int argc, char * argv[]) {
    if (argc != 3) {
        printf("the format is %s <source file> <target file>/<target dir>", argv[0]);
    }

    struct stat md1;
    if (stat(argv[1], &md1) == -1) {
        perror("error in stat function");
        exit(EXIT_FAILURE);   
    }

    if (S_ISDIR(md1.st_mode)) {
        printf("the first argument must be a file");
        exit(EXIT_FAILURE);
    }

    struct stat md2;
    int dir = 1;
    if (stat(argv[2], &md2) == -1) {
        if (errno != ENOENT) {
            perror("error in stat function");
            exit(EXIT_FAILURE);
        } else {
            dir = 0;
        } 
    }

    if (dir) {
        char* path = construct_path(argv[1], argv[2]);
        if (rename(argv[1], path) == -1) {
            perror("error in the opertaion, ensure that the first argument is" \
                    " file name and the second is new file name or directory name");
            exit(EXIT_FAILURE);
        } 
    } else {
        if (rename(argv[1], argv[2]) == -1) {
            perror("error in the opertaion, ensure that the first argument is" \
                    " file name and the second is new file name or directory name");
            exit(EXIT_FAILURE);
        }   
    }
    
    return 0;
}

char* construct_path(char* filename, char* dirname) {
    char* path = (char*)malloc(strlen(filename) + strlen(dirname) + 2);
    if (path == NULL) {
        perror("error in allocate memory");
        exit(EXIT_FAILURE);
    }

    strcpy(path, dirname);
    strcat(path, "/");
    strcat(path, filename);

    return path;
}