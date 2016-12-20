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
int consume_pattern(char *pathto, char *filename)
{
    int patt_fd;
    char pattern[PATTERN_SZ];
    char path[512];

    memset(pattern, 0, PATTERN_SZ);
    memset(path, 0, sizeof(path));

    strncpy(path, pathto, sizeof(path)-1);
    strncpy(path+strlen(pathto), "/", 1);
    strncpy(path+strlen(pathto)+1, filename, sizeof(path)-strlen(pathto)-1);
    path[strlen(pathto)+strlen(filename)+1] = 0;

    if ((patt_fd = open(path, O_RDONLY)) < 0) {
        perror("could not open world pattern");
        return 0;
    }
    read(patt_fd, pattern, PATTERN_SZ);
    if (pattern[PATTERN_SZ-1] == 1) {
        printf("\tfound reward pattern: %s\n", filename);
        health_status += HEALTH_REGEN;
    }
    printf("\tConsuming pattern %s\n", filename);
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
    printf("\t[*] Turning left\n");
    char str[96];
    str[0]=0x10; str[1]=0x10; str[2]=0x10; str[3]=0x10; str[4]=0x10;
    str[5]=0x10; str[6]=0x10; str[7]=0x10; str[8]=0x10; str[9]=0x10;
    str[10]=0x10; str[11]=0x10; str[12]=0x10; str[13]=0x10; str[14]=0x10;
    str[15]=0x10; str[16]=0x10; str[17]=0x10; str[18]=0x10; str[19]=0x10;
    str[20]=0x10; str[21]=0x10; str[22]=0x10; str[23]=0x10; str[24]=0x10;
    str[25]=0x10; str[26]=0x10; str[27]=0x10; str[28]=0x10; str[29]=0x10;
    str[30]=0x10; str[31]=0x10; str[32]=0x10; str[33]=0x10; str[34]=0x10;
    str[35]=0x10; str[36]=0x10; str[37]=0x10; str[38]=0x10; str[39]=0x10;
    str[40]=0x10; str[41]=0x10; str[42]=0x10; str[43]=0x10; str[44]=0x10;
    str[45]=0x10; str[46]=0x10; str[47]=0x10; str[48]=0x10; str[49]=0x10;
    str[50]=0x10; str[51]=0x10; str[52]=0x10; str[53]=0x10; str[54]=0x10;
    str[55]=0x10; str[56]=0x10; str[57]=0x10; str[58]=0x10; str[59]=0x10;
    str[60]=0x10; str[61]=0x10; str[62]=0x10; str[63]=0x10; str[64]=0x10;
    str[65]=0x10; str[66]=0x10; str[67]=0x10; str[68]=0x10; str[69]=0x10;
    str[70]=0x10; str[71]=0x10; str[72]=0x10; str[73]=0x10; str[74]=0x10;
    str[75]=0x10; str[76]=0x10; str[77]=0x10; str[78]=0x10; str[79]=0x10;
    str[80]=0x10; str[81]=0x10;

    return turn(dir, (char *)"L", cwd, cwdsize);
}

CPG_SECTION
DIR* turn_right(char *dir, char *cwd, int cwdsize)
{
    printf("\t[*] Turning right\n");
    char str[32];
    str[0]=0x0F; str[1]=0x0F; str[2]=0x0F; str[3]=0x0F; str[4]=0x0F;
    str[5]=0x0F; str[6]=0x0F; str[7]=0x0F; str[8]=0x0F; str[9]=0x0F;
    str[10]=0x0F; str[11]=0x0F; str[12]=0x0F; str[13]=0x0F; str[14]=0x0F;
    str[15]=0x0F; str[16]=0x0F; str[17]=0x0F; str[18]=0x0F; str[19]=0x0F;
    str[20]=0x0F; str[21]=0x0F; str[22]=0x0F; str[23]=0x0F; str[24]=0x0F;
    str[25]=0x0F; str[26]=0x0F; str[27]=0x0F; str[28]=0x0F; str[29]=0x0F;
    str[30]=0x0F; str[31]=0x0F;

    return turn(dir, (char *)"R", cwd, cwdsize);
}

// enter a directory room and scan for the resident sensory pattern.
SENSORIMOTOR_SECTION
int scan_room(DIR *curdirp, char *dir, char *cwd, int cwdsize)
{
    struct dirent *de=NULL;
    DIR *nextdirp=NULL;

    printf("[*] sensorimotor function scan_room(%s)\n", dir);
    /*
     * Get the next sensory input pattern.
     */
    printf("\t[*] calling sensory function\n");
    while ((de = readdir(curdirp)) != NULL) {
        if (de->d_type == DT_REG) {
            if (consume_pattern(dir, de->d_name) == 0) {
                //printf("[*] found pattern %s\n", de->d_name);
                return 0;
            }
        }
    }

    /*
     * Execute the next motor command. Somewhat favor left to
     * avoid 50/50 oscillating.
     */
/*
    printf("\t[*] calling motor function\n");
    if (rand()/(float)RAND_MAX < 0.65) {
        if ((nextdirp=turn_left(dir, cwd, cwdsize))==NULL)
            return 0;
    } else {
        if ((nextdirp=turn_right(dir, cwd, cwdsize))==NULL)
            return 0;
    }
*/

    if ((nextdirp=turn_left(dir, cwd, cwdsize))==NULL)
        return 0;
 
    health_status -= HEALTH_DECAY;

    closedir(nextdirp);

    return 1;
}

// explore the directory tree of rooms for files and directories
// in search of reward patterns.
int explore_world(char *dir)
{
    char cwd[512];
    DIR *curdirp = NULL;

    memset(cwd, 0, sizeof(cwd));
    getcwd(cwd, sizeof(cwd));
    curdirp = opendir(dir);

    if (scan_room(curdirp, dir, cwd, sizeof(cwd)) == 0)
        return 0;

    if (health_status <= 0) {
        printf("the primal organism has died.\n");
        return 0;
    }

    closedir(curdirp);
    if (explore_world(cwd) == 0)
        return 0;

    return 1;
}

int main(int argc, char **argv)
{
    //DIR *dirp=NULL;
    //struct dirent *de=NULL;
    char *root=(char *)"/mnt/lfs/root/ace-core/examples/LR_world/";

    srand(time(NULL)+getpid());

/*
    // begin with the pattern in the world root directory.
    if ((dirp=opendir(root)) == NULL) {
        perror("opendir() failed");
        return 0;
    }
    while ((de = readdir(dirp)) != NULL) {
        if (de->d_type == DT_REG) {
            if (consume_pattern(root, de->d_name) == 0) {
                //printf("\tfound pattern %s\n", de->d_name);
                return 0;
            }
        }
    }
*/
    printf("[*] Changing to root dir %s\n", root);
    if (chdir(root) < 0) {
        perror("chdir() failed");
        return 0;
    }
    if (!explore_world(root))
        exit(-1);

    return 0;
}

