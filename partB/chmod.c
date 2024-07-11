#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

mode_t string_to_mode(char*);

int main (int argc, char * argv[]) {
    
    if (argc != 3) {
        printf("the format is %s <filename> <mode>", argv[0]);
    }

    struct stat info;
    if (stat(argv[1], &info) == -1) {
        perror("error in stat function");
        exit(1);
    }

    if (info.st_uid == getuid() || getuid() == 0) {
        mode_t md = string_to_mode(argv[2]);
        if (chmod(argv[1], md) == -1) {
            perror("error in change mode");
            return 0;
        }
    } else {
        printf("you dont have the permission to change the mode file");
        exit(1);
    }

    return 1;
}

mode_t string_to_mode(char *mode_str) {
    if (mode_str[0] >= '0' && mode_str[0] <= '7') {
        return (mode_t)strtol(mode_str, NULL, 8);
    } else {
        mode_t mode = 0;

        if (mode_str[0] == 'r') {
            mode |= S_IRUSR;
        }
        if (mode_str[1] == 'w') {
            mode |= S_IWUSR;
        }
        if (mode_str[2] == 'x') {
            mode |= S_IXUSR;
        }
        if (mode_str[3] == 'r') {
            mode |= S_IRGRP;
        }
        if (mode_str[4] == 'w') {
            mode |= S_IWGRP;
        }
        if (mode_str[5] == 'x') {
            mode |= S_IXGRP;
        }
        if (mode_str[6] == 'r') {
            mode |= S_IROTH;
        }
        if (mode_str[7] == 'w') {
            mode |= S_IWOTH;
        }
        if (mode_str[8] == 'x') {
            mode |= S_IXOTH;
        }

        return mode;
    }
}