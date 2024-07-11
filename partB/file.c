#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

typedef struct {
    const char *magic;
    size_t magic_size;
    const char *description;
} MagicNumber;

const MagicNumber magic_numbers[] = {
    {"\x89PNG\r\n\x1A\n", 8, "PNG image"},
    {"GIF8", 4, "GIF image"},
    {"%PDF", 4, "PDF document"},
    {NULL, 0, NULL}
};

#define BUFFERSIZE 1024

void first_test(const char*);
void sec_test(const char*);
void third_test(const char*);
int is_text_file(const char*);

/*This program implemention part of the "file" command with the three
* tests. The first test try to determine the type of the file with the
* mode from stat function. The second test try to determine based the
* magic number that some files had in the firsts bytes of the file (I
* implement part from the magic numbers for the concept). The third test
* is some analyze the file by special keywords inside him (here too I
* do for the concept and not the entire options).
*/
int main (int argc, char * argv[]) {
    
    char* files[argc - 1];
    int files_count = 0;
    for (int i = 1; i < argc; i++) {
        files[files_count++] = argv[i];
    }

    for (int i = 0; i < files_count; i++) {
        first_test(files[i]);
        sec_test(files[i]);
        third_test(files[i]);
    }

    return 0;
}

void first_test(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("error in stat function");
        exit(1);
    }
    if (st.st_size == 0) {
        printf("%s: empty\n", filename);
        return;
    }
    else if (S_ISDIR(st.st_mode)) {
        printf("%s: directory\n", filename);
        return;
    }
    else if (S_ISCHR(st.st_mode)) {
        printf("%s: character device\n", filename);
        return;
    }
    else if (S_ISBLK(st.st_mode)) {
        printf("%s: block device\n", filename);
        return;
    }
    else if (S_ISFIFO(st.st_mode)) {
        printf("%s: FIFO\n", filename);
        return;
    }
    else if (S_ISLNK(st.st_mode)) {
        printf("%s: symbolick link\n", filename);
        return;
    }
    else if (S_ISSOCK(st.st_mode)) {
        printf("%s: socket\n", filename);
        return;
    }
}

int is_text_file(const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return 0;
    }

    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c != '\n' && c != '\r' && c != '\t' && !isprint(c)) {
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return 1;
}

void sec_test(const char* filename) {
    int fd;
    if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("Failed to open file");
        exit(1);
    }

    char buffer[8];
    if (read(fd, buffer, sizeof(buffer)) == -1) {
        perror("error in read from file");
        close(fd);
        exit(1);
    }
    close(fd);

    for (int i = 0; magic_numbers[i].magic != NULL; i++) {
        if (memcmp(buffer, magic_numbers[i].magic, magic_numbers[i].magic_size) == 0) {
            printf("%s: %s\n" , filename, magic_numbers[i].description);
            return;
        }
    }
}

void third_test(const char* filename) {
    int fd;
    if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("error in open file");
        exit(1);
    }

    char buffer[BUFFERSIZE];
    if (read(fd, buffer, sizeof(buffer)) == -1) {
        perror("error in read from file");
        close(fd);
        exit(1);
    }

    close(fd);

    if (strstr(buffer, "<!DOCTYPE html>") != NULL || strstr(buffer, "<html>") != NULL) {
        printf("%s: HTML document\n", filename);
        return;
    }

    if (strstr(buffer, "<xml>") != NULL) {
        printf("%s: XML file\n", filename);
        return;
    }

    if(is_text_file(filename)) {
        printf("%s: ASCII file\n", filename);
        return;
    }
}