#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib.h"

int reload_buffer (int fd, char* buffer) {
    int n_read;
    if ((n_read = read(fd, buffer, BUFFSIZE)) == -1) {
        return -1;
    }
    return n_read;
}

void close_file (int fd) {
    if (close(fd) == -1) {
        perror("error in closing file");
        exit(1);
    }
}

