#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define BUFFERSIZE 4096

void with_file(int);

int main (int argc, char* argv[]) {
    if (argc == 1) {
        int n_reads = 0, sum_n_reads = 0;
        char buff[BUFFERSIZE];
        char out_buff[BUFFERSIZE];

        while ((n_reads = read(STDIN_FILENO, buff, BUFFERSIZE)) > 0) {
            memcpy(&out_buff[sum_n_reads],buff, n_reads);
            sum_n_reads += n_reads;
        }
        if (n_reads == -1) {
                perror("error in read from stdin");
                exit(1);
        }
        if (write(STDOUT_FILENO, out_buff, sum_n_reads) == -1) {
                perror("error in write to stdin");
                exit(1);
        }
        return 0;
    }

    else if (argc >= 2) {
        int fd, i = 1, n_files = argc - 1;
        while (n_files > 0) {
            if ((fd = open(argv[i], O_RDONLY)) == -1) {
                perror("error in open file");
                exit(1);
            }
            if (argc != 2) {
                fprintf(stdout, "\n==> %s <==", argv[i]);
                fflush(stdout);
            }
            with_file(fd);
            if (close(fd) == -1) {
                perror("error in closing file");
                exit(1);
            }
            i++;
            n_files--;
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
        if ((lseek(fd, offset, SEEK_END)) == -1) {
            if (errno == EINVAL) {
                break;
            } else {
                perror("error in move on the file");
                exit(1);
            }
        }
        if (read(fd, &temp, 1) == -1) {
            perror("error in read from the file");
            exit(1);
        }
        if (temp == '\n') {
            count++;
        }
        offset--;
    }
    offset++;
    int pos_offset = (offset) * (-1);
    char* buff_out[pos_offset];
    lseek(fd, offset, SEEK_END);
    if (read(fd, buff_out, pos_offset) == -1) {
        perror("error in read from file");
        exit(1);
    }
    if (write(STDOUT_FILENO, buff_out, pos_offset) == -1) {
        perror("error in write");
        exit(1);
    }
}