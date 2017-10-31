/*
 * Represents an organism with primal intelligence that searches
 * through a filesystem for sensory patterns containing reward
 * signals.
 *
 * The sequence of sensory patterns is fixed in the sense that the
 * cpg functions are always called in a pre-determined order.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define PURESENSORY_SECTION __attribute__((section (".pure_sensory")))
#define CPG_SECTION __attribute__((section (".cpg")))
#define SENSORIMOTOR_SECTION __attribute__((section (".sensorimotor")))
// number of bits composing a sensory pattern
#define PATTERN_SZ      62262 // 144 x 144 + bmp hdr
// environmental factors for future reinforcement learning.
#define HEALTH_BASE     100
#define HEALTH_DECAY    5
#define HEALTH_REGEN    (2*HEALTH_DECAY)

#define WORLD_ROOT  "/root/ace-core/examples/abcdxbcy_world/"

long long int health_status = HEALTH_BASE;

// open and read from the fd, and parse it for a reward signal.
PURESENSORY_SECTION
int consume_pattern(char *cwd, char *filename)
{
    int patt_fd;
    char pattern[PATTERN_SZ];
    char path[512];

    memset(pattern, 0, PATTERN_SZ);
    memset(path, 0, sizeof(path));

    strncpy(path, cwd, sizeof(path)-1);
    strncpy(path+strlen(cwd), "/", 1);
    strncpy(path+strlen(cwd)+1, filename, sizeof(path)-strlen(cwd)-1);
    path[strlen(cwd)+strlen(filename)+1] = 0;

    if ((patt_fd = open(path, O_RDONLY)) < 0) {
        perror("could not open world pattern");
        return 0;
    }
    //printf("[sensory] read(%s)\n", path);
    read(patt_fd, pattern, PATTERN_SZ);
    if (pattern[PATTERN_SZ-1] == 1) {
        //printf("\tfound reward pattern: %s\n", filename);
        health_status += HEALTH_REGEN;
    }
    close(patt_fd);
    return 1;
}

// change to a new directory and open a new directory stream to read
// from.
CPG_SECTION
DIR* enter_room(char *dir, char *cwd, int cwdsize)
{
    DIR *dirp=NULL;

    //printf("[cpg] opendir(%s)\n", dir);
    if ((dirp=opendir(dir)) == NULL) {
        perror("opendir() failed");
        return 0;
    }
    if (chdir(dir) < 0) {
        perror("chdir() failed");
        return 0;
    }
    getcwd(cwd, cwdsize);

    return dirp;
}

// enter a directory room and scan for the resident sensory pattern.
SENSORIMOTOR_SECTION
DIR* scan_room(char *dir, char *cwd, int cwdsize)
{
    DIR *dirp = NULL;
    struct dirent *de;

    //printf("[sm] entering room %s\n", dir);
    if ((dirp = enter_room(dir, cwd, cwdsize)) == NULL)
        return NULL;

    health_status -= HEALTH_DECAY;

    while ((de = readdir(dirp)) != NULL) {
        if (de->d_type == DT_REG) {
            if (consume_pattern(cwd, de->d_name) == 0) {
                //printf("\tfound pattern %s\n", de->d_name);
                return 0;
            }
        }
    }

    rewinddir(dirp);
    return dirp;
}

// explore the directory tree of rooms for files and directories
// in search of reward patterns.
int explore_world(char *dir)
{
    DIR *dirp;
    struct dirent *de;
    char cwd[512];

    memset(cwd, 0, sizeof(cwd));
    if ((dirp = scan_room(dir, cwd, sizeof(cwd))) == NULL)
        return 0;

    while ((de=readdir(dirp)) != NULL) {
        if (health_status <= 0) {
            printf("the primal organism has died.\n");
            return 0;
        }
        //printf("%s: %lld\n", de->d_name, health_status);
        if (de->d_type == DT_DIR && strncmp(".", de->d_name, 1) != 0) {
            if (explore_world(de->d_name) == 0)
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
    if (!explore_world((char *)WORLD_ROOT))
        exit(-1);

    return 0;
}

