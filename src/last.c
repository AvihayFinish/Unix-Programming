#include <stdio.h>
#include <utmp.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/*
* This command print the list of the logged in to the machine.
*/

#define UTSIZE (sizeof(struct utmp))
#define NRECS 16

void show_time(long);
void show_data(struct utmp *);
int reload_buffer(int,char*);
struct utmp* next_record(int,char*);
void close_fd(int);
int open_file();

int main () {
    char buffer[UTSIZE * NRECS];
    struct utmp *record;
    int fd, n_records;
    int cur_rec = 0;

    if ((fd = open_file()) == -1) {
        perror("cannot open file");
        exit(1);
    }
    
    n_records = reload_buffer(fd, buffer);
    while (n_records) {
        record = next_record(cur_rec, buffer);
        show_data(record);
        cur_rec++;
        n_records--;
        if (cur_rec == NRECS) {
            cur_rec = 0;
        }
        if (n_records == 0) {
            n_records = reload_buffer(fd, buffer);
        }
    }

    close_fd(fd);
    return 0;
}

void show_data(struct utmp *utbufp) {

    int check = 0;
    if (utbufp->ut_type == USER_PROCESS || utbufp->ut_type == BOOT_TIME) {
        check = 1;
    }
    if (check) {
        printf("%-8.8s", utbufp->ut_name);
        printf(" ");
        printf("%-8.8s", utbufp->ut_line);
        printf(" ");
        show_time(utbufp->ut_time);
        if (utbufp->ut_host[0] != '\0') {
            printf("(%s)", utbufp->ut_host);
        }
        printf("%d", utbufp->ut_type);
        printf(" ");
        printf("\n");
    }
    
}

int reload_buffer (int fd, char* buffer) {
    int n_reads, n_records;
    if ((n_reads = read(fd, buffer, UTSIZE * NRECS)) == -1) {
        return -1;
    }
    n_records = n_reads/UTSIZE;
    return n_records;
}

struct utmp* next_record(int cur, char* buffer) {
    struct utmp* utmbcu;
    utmbcu = (struct utmp*) &buffer[cur * UTSIZE];
    return utmbcu;
}

int open_file () {
    int fd = -1;
    if ((fd = open(WTMP_FILENAME, O_RDONLY)) == -1) {
        return -1;
    }
    return fd;
}

void show_time (long timeval) {
    char* cts;
    cts = ctime(&timeval);
    printf(cts);
}

void close_fd (int fd) {
    if((close(fd)) == -1) {
        perror("error in close the file");
        exit(1);
    }
}
