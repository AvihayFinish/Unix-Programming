#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

int pass_over_files(const char*, int, int, int);
void print_info (struct stat*, const char*, int, int, int);

int main (int argc, char * argv []) {
    int each_file = 0;
    int time_m = 0;
    int readble = 0;
    char * path = NULL;

    if (argc > 5) {
        printf("the format is %s <-a|-h|--time|path>", argv[0]);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            each_file = 1;
        }
        else if (strcmp(argv[i], "-h") == 0) {
            readble = 1;
        }
        else if (strcmp(argv[i], "--time") == 0) {
            time_m = 1;
        }
        else {
            path = argv[i];
        }
    }

    if (path == NULL) {
        path = ".";
    }

    pass_over_files(path, time_m, each_file, readble);

    return 0;
}

int pass_over_files(const char* dirname, int time_m, int each_file, int readble) {
    DIR *dirptr;
    struct dirent *dirnerpt;
    int count_size_dir = 0;


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
            if (strcmp(dirnerpt->d_name, ".") == 0 || strcmp(dirnerpt->d_name, "..") == 0) {
                continue;
            }
            count_size_dir += pass_over_files(full_path, time_m, each_file, readble);          
        } else {
            count_size_dir += ((st.st_size + 1023) / 1024);

            if (each_file) {
                print_info(&st, full_path, time_m, readble, ((st.st_size + 1023) / 1024));
            }
        }

        
    }

    closedir(dirptr);

    // Print info for the current directory
    struct stat st;
    if (stat(dirname, &st) == -1) {
        perror("error in stat function");
        exit(EXIT_FAILURE);
    }
    print_info(&st, dirname, time_m, readble, count_size_dir);

    return count_size_dir;
}

void print_info (struct stat* st, const char* filename, int time_m, int readble, int the_size) {
    if (S_ISDIR(st->st_mode)) {
        if (readble) {
            printf("%dK %-12s\n", the_size, filename);
        } else {
            printf("%d %-12s\n", the_size, filename);
        }
    } else {
        if (readble) {
            if (time_m) {
                printf("%dK %.12s %-12s\n", the_size, 4+ctime(&st->st_mtime), filename);
            } else {
                printf("%dK %-12s\n", the_size, filename);
            }
        } else {
            if (time_m) {
                printf("%d %.12s %-12s\n", the_size, 4+ctime(&st->st_mtime), filename);
            } else {
                printf("%d %-12s\n", the_size, filename); 
            }
        }
    }
}