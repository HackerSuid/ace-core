#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define PATTERN_SZ  10854

#define WORLD_ROOT  "./sense_patterns"

int search_dir(char *dir)
{
    DIR *dirp;
    struct dirent *de;
    char cwd[32];

    if ((dirp=opendir(dir)) == NULL) {
        perror("opendir() failed");
        return 0;
    }
    getcwd(cwd, sizeof(cwd));
    if (chdir(dir) < 0) {
        perror("chdir() failed");
        return 0;
    }
    while ((de=readdir(dirp)) != NULL) {
        printf("%s: %d\n", de->d_name, de->d_type);
        if (de->d_type == DT_DIR && strncmp(".", de->d_name, 1) != 0) {
            if (search_dir(de->d_name) == 0)
                return 0;
            if (chdir("..") < 0) {
                perror("chdir() failed");
                return 0;
            }
        }
    }
    closedir(dirp);

    return 1;
}

int main(int argc, char **argv)
{
    int patt_fd;

    search_dir((char *)WORLD_ROOT);

    /*
    for (int i=0; i<4 && !search_complete; i++) {
        if ((patt_fd = open(patterns[i], O_RDONLY)) < 0)
            perror("could not open world pattern");
        read(patt_fd, pattern, PATTERN_SZ);
        if (pattern[PATTERN_SZ-1] == 1) {
            printf("found reward pattern: %s\n", patterns[i]);
            search_complete = true;
        }
        close(patt_fd);
    }
    */

    return 0;
}

