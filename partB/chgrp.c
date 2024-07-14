#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

/* This program take group name and file and change the group
* ownership of the file to the name group that is given.
*/
int main (int argc, char * argv[]) {

    if (argc != 3) {
        printf("the format is %s <groupName> <fileName>, try use sudo if you dont have permission", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct passwd *pwd;

    if ((pwd = getpwuid(getuid())) == NULL) {
        perror("getpwuid");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (stat(argv[2], &st) == -1) {
        perror("error in stat");
        exit(EXIT_FAILURE);
    }

    int size;
    if ((size = getgroups(0,NULL)) == -1) {
        perror("error in getgroups");
        exit(EXIT_FAILURE);
    }

    gid_t groups[size];
    if (getgroups(size, groups) == -1) {
        perror("error in getgroups");
        exit(EXIT_FAILURE);
    }

    int is_member = 0;
    struct group* grp;
    if ((grp = getgrnam(argv[1])) == NULL) {
        perror("getgrnam");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < size; i++) {
        if (grp->gr_gid == groups[i]) {
            is_member = 1;
            break;
        }
    }


    if (!is_member && geteuid() != 0 && geteuid() != st.st_uid) {
        printf("you are not have the permission to change file ownership");
        exit(EXIT_FAILURE);
    }

    if (chown(argv[2], pwd->pw_uid, grp->gr_gid) == -1) {
        perror("error in change owners");
        exit(EXIT_FAILURE);
    }

    return 0;
}