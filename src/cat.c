#include <stdio.h>
#include <stdlib.h>
#include "lib.h"
#include <unistd.h>
#include <fcntl.h>
    
/*
* this command print file to the standart input, here I create the program with library
* to get more experience with that.
*/

int main (int argc, char* argv[]) {
    
    if (argc != 2) {
        fprintf(stderr, "the format is %s and <filename>\n", argv[0]);
        exit(1);
    }

    int fd, n_reads;
    char buffer[BUFFSIZE];

    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("cannot open file");
        exit(1);
    }

    while ((n_reads = reload_buffer(fd, buffer)) > 0) {
        write(STDOUT_FILENO, buffer, n_reads);
    }

    close_file(fd);
    return 0;
}

