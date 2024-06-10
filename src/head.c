#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define BUFFERSIZE 4096

void with_file(int);

int main (int argc, char* argv[]) {
    if (argc == 1) {
        int count = 0, n_reads;
        char buff[BUFFERSIZE];

        while (count < 10) {
            if ((n_reads = read(STDIN_FILENO, buff, BUFFERSIZE)) == -1) {
                perror("error in read stdin");
                exit(1);
            }
            if (write(STDOUT_FILENO, buff, n_reads) == -1) {
                perror("error in write to stdin");
                exit(1);
            }
            count++;
        }
        return 0;
    }

    else if (argc == 2) {
        int fd;
        if ((fd = open(argv[1], O_RDONLY)) == -1) {
            perror("error in open file");
            exit(1);
        }
        with_file(fd);
        if (close(fd) == -1) {
            perror("error in closing file");
            exit(1);
        }
        return 0;
    }

    fprintf(stderr, "the format is %s and <filename> or without <filename> to stdin\n", argv[0]);
    exit(1);
}

void with_file(int fd) {
    int offset = 0, count = 0;
    char temp;
    while (count < 10) {
        if ((lseek(fd, offset, SEEK_SET)) == -1) {
            perror("error in move on the file");
            exit(1);
        }
        if (read(fd, &temp, 1) == -1) {
            perror("error in read from the file");
            exit(1);
        }
        if (temp == '\n') {
            count++;
        }
        else if (temp == EOF) {
            break;
        }
        offset++;
    }
    //size_t new_offset = offset + 1;
    char* buff_out[offset];
    lseek(fd, 0, SEEK_SET);
    if (read(fd, buff_out, offset) == -1) {
        perror("error in read from file");
        exit(1);
    }
    if (write(STDOUT_FILENO, buff_out, offset) == -1) {
        perror("error in write");
        exit(1);
    }
}