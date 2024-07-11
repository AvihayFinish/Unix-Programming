#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>


void split_string(const char*, char, char**, char**);

/* This program make the chown command with the -h option*/
int main (int argc, char * argv[]) {

    if (getuid() != 0) {
        printf("you are not have the permission to change file ownership");
    }

    if (argc != 3) {
        printf("the format is sudo %s <ownerName>:<groupName> <fileName>", argv[0]);
    }

    char * ownerName = NULL;
    char * groupName = NULL;
    char delimiter = ':';
    split_string(argv[1], delimiter, &ownerName, &groupName);

    struct passwd *pwd;

    pwd = getpwnam(ownerName);
    if (pwd == NULL) {
        perror("getpwnam");
        exit(1);
    }

    if (groupName == NULL) {
        if (chown(argv[2], pwd->pw_uid, -1) == -1) {
            perror("error in change owners");
            exit(1);
        }
        return 0;
    } else {
        if (chown(argv[2], pwd->pw_uid, pwd->pw_gid) == -1) {
            perror("error in change owners");
            exit(1);
        }
    }

    return 0;
}

/*This function take string, delimiter, and two pointers to pointers,
* and put on the pointers two strings that separated by the delimiter.
* If the delimiter not found in the string, the str copy to the first
* string and the second in null.
*/
void split_string(const char* str, char delimiter, char** firstPart, char** secondPart) {
    const char* delimiterPos = strchr(str, delimiter);
    if (delimiterPos == NULL) {
        *firstPart = strdup(str);
        *secondPart = NULL;
    } else {
        size_t firstPartLength = delimiterPos - str;
        *firstPart = (char*)malloc(firstPartLength + 1);
        strncpy(*firstPart, str, firstPartLength);
        (*firstPart)[firstPartLength] = '\0';

        *secondPart = strdup(delimiterPos + 1);
    }
}