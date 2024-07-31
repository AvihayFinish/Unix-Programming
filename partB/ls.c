#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define COLUMNS 4
#define STARTSIZEBUFFER 4096

// to hold the stat inforamation with the name of the file.
struct stat_with_name
{
    struct stat st;
    char d_name[256];
};


void do_ls(char[],int ,int, int, int, int, int, int);
void do_ls_l(char[],int, int, int, int, int, int, int,int);
int compare_strings(const void*, const void*);
int compare_times(const void*, const void*);
struct stat_with_name** sortedStats(char[], int*, int, int, int);
void show_file_info(char*, struct stat*, int, int);
void do_stat(char*, int, int);
void mode_to_letters(int, char[]);
char* uid_to_name (uid_t);
char* gid_to_name(gid_t);
int save_sub_dirs (char**, char*, char*, int);

/* This program implemention "ls" command and some of his options.
* the "ls" command, basically show the files in the current directory.
* the output is sorted by the names of the files, unless you asked other option.
* -l show extended information about the file. -U to not sort the output.
* -r to sort in reverse order. -u to sort by access time. -u and -lt
* to extended information(access time instead modification time) and sorted
* by acces time. -u and -l same as -u and -lt but sort by names. -R to show
* subdirectories recursively. -i to show the inode number of the entry.
* You also can run the command with path to another directory to get 
* their data.
*/
int main(int argc, char* argv[]) {
    int rev = 0;
    int sort = 1;
    int path = 0;
    int long_format = 0;
    int recur = 0;
    int inode_number = 0;
    int acces_time_sort = 0;
    int acces_time_details = 0;
    int dir_count = 0;
    char* directories[argc - 1];
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            long_format = 1;
        }
        else if (strcmp(argv[i], "-U") == 0) {
            sort = 0;
        }
        else if (strcmp(argv[i], "-r") == 0) {
            rev = 1;
        }
        else if (strcmp(argv[i], "-R") == 0) {
            recur = 1;
        }
        else if (strcmp(argv[i], "-u") == 0) {
            acces_time_sort = 1;
        }
        else if (strcmp(argv[i], "-lt") == 0) {
            acces_time_details = 1;
        }
        else if (strcmp(argv[i], "-i") == 0) {
            inode_number = 1;
        }
        else {
            directories[dir_count++] = argv[i];
            path = 1;
        }
    }

    // if no directories given in the input, work of the current directory.
    if (dir_count == 0) {
        directories[dir_count++] = ".";
    }

    for (int i = 0; i < dir_count; i++) {
        if (dir_count > 1) {
            printf("%s:\n", directories[i]);
        }
        // acces_time_details is the -lt option.
        if (long_format || acces_time_details) {
            do_ls_l(directories[i], rev, sort, path, recur, acces_time_details, acces_time_sort, long_format, inode_number);
        } else {
            do_ls(directories[i], rev, sort, path, recur, acces_time_details, acces_time_sort, inode_number);
        }
    }
    return 0;
}

/* This function take care about -l option, meaning show extended data 
* about the file
*/
void do_ls_l(char dirname [],int reverse, int sort, int path, int recurs, int access_time_details, int acces_time_sort, int long_format, int inode_number) {
    if (sort) {
        int i = 0;
        struct stat_with_name ** stats = sortedStats(dirname, &i, acces_time_sort, long_format, access_time_details);
        char ** subdirs = (char**)malloc(i * sizeof(char*));
        int count_subdirs = 0;
        char full_path[1024];
        if (reverse) {
            // print the files in reverse order.
            for (int j = i - 1; j >= 0; j--) {   
                if (access_time_details && acces_time_sort) {
                    snprintf(full_path, sizeof(full_path), "%s/%s", dirname, stats[j]->d_name);
                    show_file_info(stats[j]->d_name, &stats[j]->st, access_time_details, inode_number);
                } else {  
                    snprintf(full_path, sizeof(full_path), "%s/%s", dirname, stats[j]->d_name);
                    do_stat(full_path, access_time_details, inode_number);
                }

                if (recurs) {
                    if (save_sub_dirs(subdirs, dirname, stats[j]->d_name, count_subdirs) != -1) {
                        count_subdirs++;
                        // here I declare the size of subdirs to be like
                        // the size of the entrys, so its always have 
                        // inough space.
                    }
                }
                free(stats[j]);
            }
        } else {
            // print in sorted order.
            for (int j = 0; j < i; j++) {
                if (access_time_details && acces_time_sort) {
                    snprintf(full_path, sizeof(full_path), "%s/%s", dirname, stats[j]->d_name);
                    show_file_info(stats[j]->d_name, &stats[j]->st, access_time_details, inode_number);
                } else {  
                    snprintf(full_path, sizeof(full_path), "%s/%s", dirname, stats[j]->d_name);
                    do_stat(full_path, acces_time_sort, inode_number);
                }
                if (recurs) {
                    if (save_sub_dirs(subdirs, dirname, stats[j]->d_name, count_subdirs) != -1) {
                        count_subdirs++;
                    }
                }
                free(stats[j]);
            }
        }
        // call recursivaly for the sub directories.
        for (int i = 0; i < count_subdirs; i++) {
            printf("\n%s:\n", subdirs[i]);
            do_ls_l(subdirs[i], reverse, sort, path, recurs, access_time_details, acces_time_sort, long_format, inode_number);
            free(subdirs[i]);      
        }
        printf("\n");
        free(subdirs);
        free(stats);
    } else {
        // print in the order on the directory without sorted the names
        // for the -U option.
        DIR *dirptr;
        struct dirent *dirnerpt;
        char full_path[1024];
        char ** subdirs = NULL;
        int count_subdirs = 0;
        int dirs_capacity = 10;

        if ((dirptr = opendir(dirname)) == NULL) {
            fprintf(stderr, "cannot open %s\n", dirname);
        } else {
            subdirs = (char**)malloc(dirs_capacity * sizeof(char*));
            while ((dirnerpt = readdir(dirptr)) != NULL) {
                snprintf(full_path, sizeof(full_path), "%s/%s", dirname, dirnerpt->d_name);
                do_stat(full_path, access_time_details, inode_number);
                
                if (recurs) {
                    if (save_sub_dirs(subdirs, dirname, dirnerpt->d_name, count_subdirs) != -1) {
                        count_subdirs++;
                        // if memory in subdirs is over - reallocate.
                        if (count_subdirs == dirs_capacity) {
                            dirs_capacity += 10;
                            subdirs = realloc(subdirs, dirs_capacity * sizeof(char*));
                            if (subdirs == NULL) {
                                perror("error in realloc");
                                exit(1);
                            }
                        }
                    }
                }
            }
            closedir(dirptr);
        }
        // call recursivaly for the sub directories.
        for (int i = 0; i < count_subdirs; i++) {
            printf("\n%s:\n", subdirs[i]);
            do_ls_l(subdirs[i], reverse, sort, path, recurs, access_time_details, acces_time_sort, long_format, inode_number);
            free(subdirs[i]);      
        }
        printf("\n");
        free(subdirs);
    }
}

/* This function take care about the regular command*/
void do_ls(char dirname [], int reverse, int sort, int path, int recurs, int access_time_details, int access_time_sort, int inode_number) {
    if (sort) {
        int i = 0;
        struct stat_with_name ** stats = sortedStats(dirname, &i, access_time_sort, 0, access_time_details);
        char ** subdirs = (char**)malloc(i * sizeof(char*));
        int count_subdirs = 0;
        int col = 0;
        if (reverse) {
            // print the names in reverse order.
            for (int j = i - 1; j >= 0; j--) {
                if (inode_number) {
                    printf("%-10ld", stats[j]->st.st_ino);
                }
                printf("%-20s", stats[j]->d_name);
                col++;
                if (col == COLUMNS) {
                    printf("\n");
                    col = 0;
                }
                if (recurs) {
                    if (save_sub_dirs(subdirs, dirname, stats[j]->d_name, count_subdirs) != -1) {
                        count_subdirs++;
                        // here I declare the size of subdirs to be like
                        // the size of the entrys, so its always have 
                        // inough space.
                    }
                }
                free(stats[j]);
            }
        } else {
            // print in sorted order
            for (int j = 0; j < i; j++) {
                if (inode_number) {
                    printf("%-10ld", stats[j]->st.st_ino);
                }
                printf("%-20s", stats[j]->d_name);
                col++;
                if (col == COLUMNS) {
                    printf("\n");
                    col = 0;
                }
                if (recurs) {
                    if (save_sub_dirs(subdirs, dirname, stats[j]->d_name, count_subdirs) != -1) {
                        count_subdirs++;
                    }
                }
                free(stats[j]);
            }
        }
        // call recursivaly for the sub directories.    
        for (int i = 0; i < count_subdirs; i++) {
            printf("\n%s:\n", subdirs[i]);
            do_ls(subdirs[i], reverse, sort, path, recurs, access_time_details, access_time_sort, inode_number);
            free(subdirs[i]);      
        }
        printf("\n");
        free(subdirs);
        free(stats);
    } else {
        // print in the order on the directory without sorted the names
        // for the -U option.
        DIR *dirptr;
        struct dirent *dirnerpt;
        int col = 0;
        char ** subdirs = NULL;
        int count_subdirs = 0;
        int dirs_capacity = 10;

        if ((dirptr = opendir(dirname)) == NULL) {
            fprintf(stderr, "cannot open %s\n", dirname);
        } else {
            subdirs = (char**)malloc(dirs_capacity * sizeof(char*));

            while ((dirnerpt = readdir(dirptr)) != NULL) {
                if (inode_number) {
                    struct stat st;
                    if (stat(dirnerpt->d_name, &st) == -1) {
                        perror("error in stat function");
                        exit(EXIT_FAILURE);
                    }
                    printf("%-10ld ", st.st_ino);
                }

                printf("%-20s", dirnerpt->d_name);
                col++;
                if (col == COLUMNS) {
                    col = 0;
                    printf("\n");
                }

                if (recurs) {
                    if (save_sub_dirs(subdirs, dirname, dirnerpt->d_name, count_subdirs) != -1) {
                        count_subdirs++;
                        if (count_subdirs == dirs_capacity) {
                            dirs_capacity += 10;
                            subdirs = realloc(subdirs, dirs_capacity * sizeof(char*));
                            if (subdirs == NULL) {
                                perror("error in realloc");
                                exit(1);
                            }
                        }
                    }
                }
            }

            // call recursivaly for the sub directories.
            for (int i = 0; i < count_subdirs; i++) {
                printf("\n%s:\n", subdirs[i]);
                do_ls(subdirs[i], reverse, sort, path, recurs, access_time_details, access_time_sort, inode_number);
                free(subdirs[i]);      
            }
            free(subdirs);
            printf("\n");
            closedir(dirptr);
        }
    }
}

/* To save subdirectories for -R option*/
int save_sub_dirs (char ** subdirs,char * dirname, char * name, int index) {
    // if the entry is the current directory or the father directory,
    // dont add them to the subdirs array.
    if (strcmp(".", name) == 0 || strcmp("..", name) == 0) {
        return -1;
    }
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", dirname, name);
    struct stat st;
    if (stat(full_path, &st) == -1) {
        perror("error in stat function");
        exit(1);
    } else {
        if (S_ISDIR(st.st_mode)) {
            // strdup do malloc too, so she return pointer to memory in the
            // heap that already have the string.
            subdirs[index] = strdup(full_path);
        } else {
            return -1;
        }
    }
    return 0;
}

/*This function get to strings and compare between them.*/
int compare_strings(const void* t1, const void* t2) {
    struct stat_with_name *stat1 = *(struct stat_with_name **)t1;
    struct stat_with_name *stat2 = *(struct stat_with_name **)t2;
    return strcmp(stat1->d_name, stat2->d_name);
}

/* This function take care about sorted the information we get
* by the option that we pass to it.
*/
struct stat_with_name ** sortedStats (char dirname [], int* ptr, int access_time_sort, int long_format, int access_time_details) {
    DIR *dirptr;
    struct dirent *dirnerpt;
    struct stat_with_name ** stats;
    size_t much_double_alloc = 1;

    stats = (struct stat_with_name**)calloc(much_double_alloc * STARTSIZEBUFFER, sizeof(struct stat_with_name*));
    if (stats == NULL) {
        printf("error in allocate memory");
        exit(1);
    }

    if ((dirptr = opendir(dirname)) == NULL) {
        fprintf(stderr, "cannot open %s\n", dirname);
        exit(1);
    }

    int i = 0;
    while ((dirnerpt = readdir(dirptr)) != NULL) {
        stats[i] = (struct stat_with_name*)malloc(sizeof(struct stat_with_name));
        size_t nameLen = sizeof(dirnerpt->d_name) + 1;
        if(stats[i] == NULL) {
            printf("error in allocate memory");
            for (int j = 0; j < i; j++) {
                free(stats[j]);
            }
            free(stats);
            exit(1);
        }

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, dirnerpt->d_name);

        if (stat(fullpath, &stats[i]->st) == -1) {
            perror("stat");
            free(stats[i]);
            continue; 
        }

        strncpy(stats[i]->d_name, dirnerpt->d_name, nameLen);

        i++;
        
        if (i > (much_double_alloc * STARTSIZEBUFFER)){
            // if we have more subdirs from the memory that we allocate 
            // first, then reallocate.
            much_double_alloc++;
            stats = (struct stat_with_name**)realloc(stats, much_double_alloc * STARTSIZEBUFFER);
            if (stats == NULL) {
                printf("error in realloc memory");
                exit(1);
            }
        }
    }

    if (access_time_sort && long_format) {
        qsort(stats, i, sizeof(struct stat_with_name*), compare_strings);
    } else if (access_time_sort || access_time_details) {
        qsort(stats, i, sizeof(struct stat_with_name*), compare_times);
    } else {
        qsort(stats, i, sizeof(struct stat_with_name*), compare_strings);
    }
   
    closedir(dirptr);
    *ptr = i;
    return stats;
}

/*This function compare between two access times of files.*/
int compare_times (const void* t1, const void* t2) {
    struct stat_with_name *stat1 = *(struct stat_with_name **)t1;
    struct stat_with_name *stat2 = *(struct stat_with_name **)t2;

    if (stat1->st.st_atime == stat2->st.st_atime) {
        return 0;
    } else if (stat1->st.st_atime > stat2->st.st_atime) {
        return -1;
    }
    return 1;
}

/* This function do the job for regular -l option*/
void do_stat (char* name, int access_time_details, int inode_number) {
    struct stat info;
    if (stat(name, &info) == -1) {
        perror(name);
        exit(1);
    }
    show_file_info(name, &info, access_time_details, inode_number);
}

void show_file_info (char* filename, struct stat* info_p, int access_time_details, int inode_number) {
    char *uid_to_name(), *gid_to_name(), *ctime();
    void mode_to_letters();
    char modest[11];

    mode_to_letters(info_p->st_mode, modest);

    printf("%s ", modest);
    printf("%4d ", (int) info_p->st_nlink);
    if (inode_number) {
        printf("%4d ", (int) info_p->st_ino);
    }
    printf("%-8s ", uid_to_name(info_p->st_uid));
    printf("%-8s ", gid_to_name(info_p->st_gid));
    printf("%8ld ", (long)info_p->st_size);
    if (access_time_details) {
        printf("%.12s ", 4+ctime(&info_p->st_atime));    
    }
    else {
        printf("%.12s ", 4+ctime(&info_p->st_mtime));
    }
    printf("%s\n ", filename);
}

/* This function convert the bits that have the metadata about the
* file to string.
*/
void mode_to_letters(int md, char str []) {
    int setU = 0, setG = 0, stic = 0;
    if (S_ISUID & md) {
        setU = 1;
    }
    if (S_ISGID & md) {
        setG = 1;
    }
    if (__S_ISVTX & md) {
        stic = 1;
    }

    strcpy(str, "----------");

    if (S_ISDIR(md)) 
        str[0] = 'd';
    if (S_ISCHR(md))
        str[0] = 'c';
    if (S_ISBLK(md))
        str[0] = 'd';

    if (S_IRUSR & md)
        str[1] = 'r';
    if (S_IWUSR & md)
        str[2] = 'w';
    if (S_IXUSR & md) {
        if (setU) {
            str[3] = 's';
        } else {
            str[3] = 'x';
        }
    } else if (setU) {        
        str[3] = 'S';
    }

    if (S_IRGRP & md)
        str[4] = 'r';
    if (S_IWGRP & md)
        str[5] = 'w';
    if (S_IXGRP & md) {
        if (setG) {
            str[6] = 's';   
        } else {
            str[6] = 'x';
        }
    } else if (setG) {  
        str[6] = 'S';
    }

    if (S_IROTH & md)
        str[7] = 'r';
    if (S_IWOTH & md)
        str[8] = 'w';
    if (S_IXOTH & md) {
        if (stic) {
            str[9] = 't';    
        } else {
            str[9] = 'x';
        }
    } else if (stic) {
        str[9] = 'T';
    }
}

/* This function get user ID and return user name*/
char* uid_to_name (uid_t uid) {
    struct passwd *pw_ptr;
    static char namest[10];

    if ((pw_ptr = getpwuid(uid)) == NULL) {
        sprintf(namest, "%d", uid);
        return namest;
    }

    return pw_ptr->pw_name;
}

/* This function get group ID and return group name*/
char* gid_to_name (gid_t gid) {
    struct group *pw_ptr;
    static char namegr[10];

    if ((pw_ptr = getgrgid(gid)) == NULL) {
        sprintf(namegr, "%d", gid);
        return namegr;
    }

    return pw_ptr->gr_name;
}
