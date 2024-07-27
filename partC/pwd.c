#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

ino_t get_inode(char*);
void print_path_to(ino_t);
void inum_to_name(ino_t, char*, int);

/* This program return the current path of the procces that run it.*/
int main () {
    print_path_to(get_inode("."));
    putchar('\n');
    return 0;
}

void print_path_to(ino_t this_inode) {
    ino_t my_inode;
    char its_name[BUFSIZ];
    if (get_inode("..") != this_inode) {
        if (chdir("..") == -1) {
            perror("error in change dir");
            exit(EXIT_FAILURE);
        }
        inum_to_name(this_inode, its_name, BUFSIZ);
        my_inode = get_inode(".");
        print_path_to(my_inode);
        printf("/%s", its_name);
    }
}

void inum_to_name(ino_t inode_to_find, char* nambuf, int buflen) {
    DIR *dir_ptr;
    struct dirent *direntpt;
    if ((dir_ptr = opendir(".")) == NULL) {
        perror("error in open dir");
        exit(EXIT_FAILURE);
    }

    while ((direntpt = readdir(dir_ptr)) != NULL) {
        if (direntpt->d_ino == inode_to_find) {
            strncpy(nambuf, direntpt->d_name, buflen);
            nambuf[buflen - 1] = "\0";
            closedir(dir_ptr);
            return;
        }
    }
    fprintf(stderr, "error to find inode %ld\n", inode_to_find);
    exit(EXIT_FAILURE);
}

ino_t get_inode(char* fname) {
    struct stat st;
    if (stat(fname, &st) == -1) {
        perror("error in stat function");
        exit(EXIT_FAILURE);
    }
    return st.st_ino;
}

