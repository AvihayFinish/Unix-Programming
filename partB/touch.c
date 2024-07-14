#include <stdio.h>
#include <stdlib.h>
#include <utime.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

void chnage_file_times(const char*, int);

/* This program modify the acces and modify times of the file
* to the current time. if the file not exist the program create one
* with the name that provided, unless the -c flag is given.
*/
int main (int argc, char * argv[]) {

    if (argc > 3) {
        printf("the format is %s <option -c flag> <filename", argv[0]);
    }

    int checkC = 0;
    char * filename;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            checkC = 1;
        } else {
            filename = argv[i];
        }
    }

    chnage_file_times(filename, checkC);

    return 0;
}

void chnage_file_times(const char* filename, int checkC) {
    if (checkC) {
        if (access(filename, F_OK) == 0) {
            return;
        }
    } else if (access(filename, F_OK) == -1) {
        if (open(filename, O_CREAT) == -1) {
            perror("error in create the file");
            exit(EXIT_FAILURE);
        }
    } else {

        time_t curr;
        if (time(&curr) == -1) {
            perror("error in time function");
            exit(EXIT_FAILURE);
        }

        struct utimbuf times;
        times.actime = curr;
        times.modtime = curr;
        if (utime(filename, &times) == -1) {
            perror("error in modifing times");
            exit(EXIT_FAILURE);
        }
    }
}