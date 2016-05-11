/*
 * Represents an organism with primal intelligence that searches
 * through a filesystem for sensory patterns containing reward
 * signals.
 *
 * The sequence of sensory patterns is arbitrary in the sense that the
 * cpg functions are called in no particular order.
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
#include <time.h>

#define PURESENSORY_SECTION __attribute__((section (".pure_sensory")))
#define CPG_SECTION __attribute__((section (".cpg")))
#define SENSORIMOTOR_SECTION __attribute__((section (".sensorimotor")))
// number of bits composing a sensory pattern
#define PATTERN_SZ      10854
// environmental factors for future reinforcement learning.
#define HEALTH_BASE     100
#define HEALTH_DECAY    5
#define HEALTH_REGEN    (2*HEALTH_DECAY)

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
    read(patt_fd, pattern, PATTERN_SZ);
    if (pattern[PATTERN_SZ-1] == 1) {
        printf("\tfound reward pattern: %s\n", filename);
        health_status += HEALTH_REGEN;
    }
    close(patt_fd);
    return 1;
}

// change to a new directory and open a new directory stream to read
// from.
DIR* turn(char *dir, char *direction, char *cwd, int cwdsize)
{
    DIR *dirp=NULL;
    char nxtdir[512];

    unsigned int dirlen = strlen(dir);
    strncpy(nxtdir, dir, sizeof(nxtdir)-1);
    if (dir[dirlen-1] != '/') {
        strncpy(nxtdir+dirlen, "/", 1);
        dirlen++;
    }
    strncpy(nxtdir+dirlen, direction, 2);
    nxtdir[dirlen+2] = 0;


    if ((dirp=opendir(nxtdir)) == NULL) {
        perror("opendir() failed");
        return 0;
    }
    if (chdir(nxtdir) < 0) {
        perror("chdir() failed");
        return 0;
    }
    getcwd(cwd, cwdsize);

    return dirp;
}

CPG_SECTION
DIR* turn_left(char *dir, char *cwd, int cwdsize)
{
    return turn(dir, (char *)"L/", cwd, cwdsize);
}

CPG_SECTION
DIR* turn_right(char *dir, char *cwd, int cwdsize)
{
    return turn(dir, (char *)"R/", cwd, cwdsize);
}

// enter a directory room and scan for the resident sensory pattern.
SENSORIMOTOR_SECTION
int scan_room(char *dir, char *cwd, int cwdsize)
{
    DIR *dirp = NULL;
    struct dirent *de;

    // somewhat favor left to avoid 50/50 oscillating.
    if (rand()/(float)RAND_MAX < 0.75) {
        if ((dirp=turn_left(dir, cwd, cwdsize))==NULL)
            return 0;
    } else {
        if ((dirp=turn_right(dir, cwd, cwdsize))==NULL)
            return 0;
    }
 
    health_status -= HEALTH_DECAY;

    while ((de = readdir(dirp)) != NULL) {
        if (de->d_type == DT_REG) {
            if (consume_pattern(cwd, de->d_name) == 0) {
                //printf("\tfound pattern %s\n", de->d_name);
                return 0;
            }
        }
    }

    closedir(dirp);
    return 1;
}

// explore the directory tree of rooms for files and directories
// in search of reward patterns.
int explore_world(char *dir)
{
    DIR *dirp;
    struct dirent *de;
    char cwd[512];

    memset(cwd, 0, sizeof(cwd));
    if (scan_room(dir, cwd, sizeof(cwd)) == 0)
        return 0;

    if (health_status <= 0) {
        printf("the primal organism has died.\n");
        return 0;
    }

    if (explore_world(cwd) == 0)
        return 0;

    return 1;
}

int main(int argc, char **argv)
{
    DIR *dirp=NULL;
    struct dirent *de=NULL;

    srand(time(NULL)+getpid());

    // begin with the pattern in the world root directory.
    if ((dirp=opendir(argv[1])) == NULL) {
        perror("opendir() failed");
        return 0;
    }
    while ((de = readdir(dirp)) != NULL) {
        if (de->d_type == DT_REG) {
            if (consume_pattern(argv[1], de->d_name) == 0) {
                //printf("\tfound pattern %s\n", de->d_name);
                return 0;
            }
        }
    }

    if (!explore_world(argv[1]))
        exit(-1);

    return 0;
}
