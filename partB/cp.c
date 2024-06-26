#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>


#define BUFFERSIZE 4096
#define COPYMODE 0644

int declare_file_or_dir(char*);
void copy_file_to_dir(char*, char*, int, int, int);
void copy_file(char*, char*, int, int);
void copy_dir_to_dir(char*, char*, int, int);
void ask_overise (char*);

int main (int argc, char* argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file> <optionals flag> <output_file>\n", argv[0]);
        exit(1);
    }

    int dir_or_file[argc -1];
    int checkI = 0;
    int checkP = 0;
    char* names[argc - 1];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            checkI = 1;
        } else if (strcmp(argv[i], "-p") == 0) {
            checkP = 1;
        } else {
            dir_or_file[i - 1] = declare_file_or_dir(argv[i]);
            names[i - 1] = argv[i];
        }
    }

    if (dir_or_file[0] == 0 && dir_or_file[1] == 0) {
        copy_file(names[0], names[1], checkI, checkP);
    } else if (dir_or_file[0] == 1 && dir_or_file[1] == 0) {
        copy_file_to_dir(names[1], names[0], checkI, checkP, 0);
    } else if (dir_or_file[0] == 0 && dir_or_file[1] == 1) {
        copy_file_to_dir(names[0], names[1], checkI, checkP, 0);
    } else {
        copy_dir_to_dir(names[0], names[1], checkI, checkP);
    }

    return 0;
}

void ask_overise(char* name) {
    char check [10];
    if ((access(name, F_OK)) == 0) {
            printf("%s is exists, do you wnat to overise him y/n?", name);
            if ((fgets(check, sizeof(check), stdin)) != NULL) {
                check[1] = '\0';
                if (strcmp(check, "n") == 0 || strcmp(check, "N") == 0) {
                    printf("we not tuch the file");
                    exit(1);
                }
            }
        }
}

int declare_file_or_dir(char* name) {
    struct stat st;
    if (access(name, F_OK) != 0) {
        return 0;
    }
    if (stat(name, &st) == -1) {
        perror("error in stat function");
        return -1;
    }
    if (S_ISDIR(st.st_mode)) 
        return 1;
    if (S_ISREG(st.st_mode))
        return 0;
    return -1;
}

void copy_file(char* name1, char* name2, int checkI, int checkP) {
    if (checkI) {
        ask_overise(name2);
    }

    int in_fd, out_fd, n_chars;
    char buffer[BUFFERSIZE];

    if ((in_fd = open(name1, O_RDONLY)) == -1) {
        perror("cannot open file 1");
        exit(1);
    }  else if ((out_fd = open(name2, O_CREAT | O_WRONLY, COPYMODE)) == -1) {
        perror("cannot open file 2");
        exit(1);
    }
    
    while ((n_chars = read(in_fd, buffer, BUFFERSIZE)) > 0) {
        if ((write(out_fd, buffer, n_chars)) != n_chars) {
            perror("cannot write to file");
            exit(1);
        }
    }
    if (checkP) {
        close(out_fd);
        struct stat st;
        if ((stat(name1, &st)) == -1) {
            perror("error in stat function");
            exit(1);
        }
        else if ((out_fd = open(name2, O_WRONLY, st.st_mode)) == -1) {
            perror("cannot open file 2");
            exit(1);
        }
        struct utimbuf times;
        times.actime = st.st_atime;
        times.modtime = st.st_mtime;
        if (utime(name2, &times) == -1) {
            perror("error in modifing times");
            exit(1);
        }
        printf("here \n");
        printf("%s", name2);
    }

    if (close(in_fd) == -1 || close(out_fd) == -1) {
        perror("error in closing files");
        exit(1);
    }
}

void copy_file_to_dir(char* name1, char* name2, int checkI, int checkP, int option) {
    char* file_path = malloc(sizeof(name1) + sizeof(name2) +2);
    if (file_path == NULL) {
        perror("error in allocate memory");
        exit(1);
    } 
    if (option) {
        copy_file(name1, name2, checkI, checkP);
        return;
    }
    strcpy(file_path, name2);
    strcat(file_path, "/");
    strcat(file_path, name1);

    printf("%s %s", name1, file_path);

    copy_file(name1, file_path, checkI, checkP);
}

void copy_dir_to_dir(char* name1, char* name2, int checkI, int checkP) {
    DIR* dirptr;
    struct dirent* direntpt;

    if ((dirptr = opendir(name1)) == NULL) {
        perror("error in open dir");
    } else {
        int i = -1;
        char full_path1[1024];
        char full_path2[1024];
        while ((direntpt = readdir(dirptr)) != NULL) {
            snprintf(full_path1, sizeof(full_path1), "%s/%s", name1, direntpt->d_name);
            snprintf(full_path2, sizeof(full_path2), "%s/%s", name2, direntpt->d_name);
            if (declare_file_or_dir(full_path1) == 0) {
                copy_file_to_dir(full_path1, full_path2, checkI, checkP, 1);
            } else {
                continue;
            }
        }
    }
    closedir(dirptr);
}
