#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 4096
#define COPYMODE 0644

int main (int argc, char* argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file> <optional flag> <output_file>\n", argv[0]);
        exit(1);
    }

    char check [10];
    int checkI = 0;

    if (argc == 4) {
        if (strcmp(argv[2], "-i") == 0) {
            checkI = 1;
            if ((access(argv[3], F_OK)) == 0) {
                printf("this file is exists, if you wnat to overise him y/n?");
                if ((fgets(check, sizeof(check), stdin)) != NULL) {
                    check[1] = '\0';
                    if (strcmp(check, "n") == 0 || strcmp(check, "N") == 0) {
                        printf("we not tuch the file");
                        exit(1);
                    }
                }
            }
        }
    }

    int in_fd, out_fd, n_chars;
    char buffer[BUFFERSIZE];

    if ((in_fd = open(argv[1], O_RDONLY)) == -1) {
        perror("cannot open file 1");
        exit(1);
    }

    if (checkI) {
        if  ((out_fd = open(argv[3], O_CREAT | O_WRONLY, COPYMODE)) == -1) {
            perror("cannot open file 2");
            exit(1);
        }
    }
    else if ((out_fd = open(argv[2], O_CREAT | O_WRONLY, COPYMODE)) == -1) {
        perror("cannot open file 2");
        exit(1);
    }

    while ((n_chars = read(in_fd, buffer, BUFFERSIZE)) > 0) {
        if ((write(out_fd, buffer, n_chars)) != n_chars) {
            perror("cannot write to file");
            exit(1);
        }
    }

    if (close(in_fd) == -1 || close(out_fd) == -1) {
        perror("error in closing files");
        exit(1);
    }

    return 0;
}