#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

/* To recognize the number that given as an input*/
enum comparison {
    EXACT,
    GREATER,
    LESS
};

/* To hold the number and the comparison*/
typedef struct comparison_result {
    enum comparison type;
    long value;
} cmp;

cmp* the_number(const char*);
void pass_over_files(const char*, int, int, int, cmp*, char*);
int save_sub_dirs (char**, const char*, const char*, int);
void f_empty(struct stat*, const char*);
void f_inum(struct stat*, cmp*, const char*);
void f_links(struct stat*, cmp*, const char*);
void f_type(mode_t md, char*, const char*);

/* This program make the find command. for this time I implement only the
* inum, links, empty and type test without options. from time to time I will
* add more tests and options to this program.
*/
int main (int argc, char * argv[]) {
    char* start_point = ".";
    int empty = 0;
    int inum = 0;
    int links = 0;
    int n = -1;
    char * type = NULL;
    cmp* rs = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-empty") == 0) {
            empty = 1;
            break;
        }
        else if (strcmp(argv[i], "-inum") == 0) {
            inum = 1;
            rs = the_number(argv[++i]);
        }
        else if (strcmp(argv[i], "-links") == 0) {
            links = 1;
            rs = the_number(argv[++i]);
        }
        else if (strcmp(argv[i], "-type") == 0) {
            type = argv[++i];
        }
        else {
            start_point = argv[i];
        }
    }

    pass_over_files(start_point, empty, inum, links, rs, type);

    return 0;
}

/* This function take care of decode the number that the user gave
* and the sign that given with it.
*/
cmp* the_number(const char* str) {
    cmp* rs;
    rs = (cmp*)malloc(sizeof(cmp));
    
    if (str[0] == '+') {
        rs->type = GREATER;
        rs->value = strtol(str + 1, NULL, 10);
    }
    else if (str[0] == '-') {
        rs->type = LESS;
        rs->value = strtol(str + 1, NULL, 10);
    }
    else {
        rs->type = EXACT;
        rs->value = strtol(str, NULL, 10);
    }
    return rs;
}

/* This function pass over the enrtys of the given directory and call the
*  test for the files by the ints that the function get.
*/
void pass_over_files(const char* dirname, int empty, int inum, int links, cmp* rs, char* type) {
    DIR *dirptr;
    struct dirent *dirnerpt;
    int count_subdirs = 0;
    int dirs_capacity = 10;

    char ** subdirs = (char**)malloc(dirs_capacity * sizeof(char*));

    if ((dirptr = opendir(dirname)) == NULL) {
        fprintf(stderr, "cannot open %s\n", dirname);
        exit(1);
    }

    while ((dirnerpt = readdir(dirptr)) != NULL) {
        // pass to stat full path to the case that the user gave start point
        // that is not the directory work of the process and for the
        // recursively calls.
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dirname, dirnerpt->d_name);

        struct stat st;
        if (stat(full_path, &st) == -1) {
            perror("error in stat function");
            exit(EXIT_FAILURE);
        }
        if (S_ISDIR(st.st_mode)) {
            if (save_sub_dirs(subdirs, dirname, dirnerpt->d_name, count_subdirs) != -1) {
                count_subdirs++;
                if (count_subdirs == dirs_capacity) {
                    dirs_capacity += 10;
                    subdirs = realloc(subdirs, dirs_capacity * sizeof(char*));
                    if (subdirs == NULL) {
                        perror("error in realloc");
                        exit(1);
                    }
                }
            }
        }

        if (empty) {
            f_empty(&st, dirnerpt->d_name);
        }
        else if (inum){
            f_inum(&st, rs, dirnerpt->d_name);
        }
        else if (links) {
            f_links(&st, rs, dirnerpt->d_name);
        } 
        else if (type != NULL) {
            f_type(st.st_mode, type, dirnerpt->d_name);
        }      
    }

    // if the current directory have a sub directories, call recursively
    // to this function with the sub directories.
    if (count_subdirs > 0) {
        for (int i = 0; i < count_subdirs; i++) {
            printf("\n*** %s ***\n", subdirs[i]);
            pass_over_files(subdirs[i], empty, inum, links, rs, type);
            free(subdirs[i]);
        }
    }

    free(subdirs);
    closedir(dirptr);
}

/* This function get array of pointers and add to him the entry if it is
* a directory too. return 0 in sucsses, else -1.
*/
int save_sub_dirs (char ** subdirs, const char * dirname, const char * name, int index) {
    if (strcmp(".", name) == 0 || strcmp("..", name) == 0) {
        return -1;
    }
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", dirname, name);
    // strdup do malloc too, so she return pointer to memory in the
    // heap that already have the string.
    subdirs[index] = strdup(full_path);
    return 0;
}

/* This function check if the file is empty.*/
void f_empty(struct stat* st, const char* filename) {
    if (st->st_size == 0) {
        printf("%s\n", filename);
    }
}

/* This function check the inode number of the file and print him 
* according to the number that the user gave as input.
*/
void f_inum(struct stat* st, cmp* rs, const char* filename) {
    switch (rs->type) {
    case EXACT:
        if (st->st_ino == rs->value) {
            printf("%s %-8ld\n", filename, st->st_ino);
        }
        break;
    case GREATER:
        if (st->st_ino > rs->value) {
            printf("%s %-8ld\n", filename, st->st_ino);
        }
        break;
    case LESS:
        if (st->st_ino < rs->value) {
            printf("%s %-8ld\n", filename, st->st_ino);
        }
        break;
    }
}

/* This function check the number of the links to the file and print him
* according to the number that the user gave as input.
*/
void f_links(struct stat* st, cmp* rs, const char* filename) {
    switch (rs->type) {
    case EXACT:
        if (st->st_nlink == rs->value) {
            printf("%s %-8ld\n", filename, st->st_nlink);
        }
        break;
    case GREATER:
        if (st->st_nlink > rs->value) {
            printf("%s %-8ld\n", filename, st->st_nlink);
        }
        break;
    case LESS:
        if (st->st_nlink < rs->value) {
            printf("%s %-8ld\n", filename, st->st_nlink);
        }
        break;
    }
}

/* This function print the file name if the file is from the type that
* given by the user.
*/
void f_type(mode_t md, char* type, const char* filename) {
    if (strcmp(type, "d") == 0) {
        if (S_ISDIR(md)) 
            printf("%s\n", filename);
    }
    else if (strcmp(type, "c") == 0) {
        if (S_ISCHR(md))
            printf("%s\n", filename);
    }
    else if (strcmp(type, "f") == 0) {
        if (S_ISREG(md))
           printf("%s\n", filename);
    }
    else if (strcmp(type, "l") == 0) {
        if (S_ISLNK(md))
            printf("%s\n", filename);
    }
    else if (strcmp(type, "b") == 0) {
        if (S_ISBLK(md))
            printf("%s\n", filename);
    }
    else if (strcmp(type, "p") == 0) {
        if (S_ISFIFO(md))
            printf("%s\n", filename);
    }
    else if (strcmp(type, "s") == 0) {
        if (S_ISSOCK(md))
            printf("%s\n", filename);
    } 
    else {
        printf("the option for type is: d-directory, f-regular file,\n"
                "c-character, l-symbolic link, b-block, p-pipe, s-socket");
        exit(EXIT_FAILURE);
    }
}
