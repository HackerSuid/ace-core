#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define PATTERN_SZ  10854

const char *patterns[4] = {
    "/root/ace/examples/sense_patterns/north/patt.bmp",
    "/root/ace/examples/sense_patterns/east/patt.bmp",
    "/root/ace/examples/sense_patterns/south/patt.bmp",
    "/root/ace/examples/sense_patterns/west/patt.bmp"
};

int main(int argc, char **argv)
{
    bool search_complete = false;
    int patt_fd;
    char pattern[PATTERN_SZ];

    for (int i=0; i<4 && !search_complete; i++) {
        if ((patt_fd = open(patterns[i], O_RDWR, 640)) < 0)
            perror("could not open world pattern");
        read(patt_fd, pattern, PATTERN_SZ);
        if (pattern[PATTERN_SZ-1] == 1) {
            printf("found reward pattern: %s\n", patterns[i]);
            search_complete = true;
        }
        close(patt_fd);
    }

    return 0;
}

