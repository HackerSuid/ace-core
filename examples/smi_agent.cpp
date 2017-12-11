/*
 * this code is used with the SMI algorithm to explore
 * a filesystem of objects.
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
#include <signal.h>

#define PURESENSORY_SECTION __attribute__((section (".pure_sensory")))
#define CPG_SECTION __attribute__((section (".cpg")))
#define SENSORIMOTOR_SECTION __attribute__((section (".sensorimotor")))
// number of bits composing a sensory pattern
#define PATTERN_SZ      10854

void reset_signal_callback(int signum)
{
    // for the ElfCodec to know about a reset of
    // sequence or object.
    //printf("agent signaled a reset.\n");
}

// open and read from the fd, and parse it for a reward signal.
PURESENSORY_SECTION
int acquire_pattern(char *filename)
{
    int patt_fd;
    char pattern[PATTERN_SZ];
    char cwd[1024];
    char path[1024];

    memset(cwd, 0, sizeof(cwd));
    memset(path, 0, sizeof(path));
    memset(pattern, 0, PATTERN_SZ);
    getcwd(cwd, sizeof(cwd));

    strncpy(path, cwd, sizeof(path)-1);
    char *fileptr = path + strlen(path);
    strncpy(fileptr, (char *)"/", 1);
    fileptr++;
    strncpy(fileptr, filename, fileptr-(&path[0])-1);
    unsigned int pathlen = strlen(path);
    path[pathlen] = 0;

    //printf("\tConsuming pattern %s\n", path);
    if ((patt_fd = open(path, O_RDONLY)) < 0) {
        perror("could not open senaory pattern");
        return 0;
    }
    //printf("\nReading pattern\n");
    read(patt_fd, pattern, PATTERN_SZ);
    //printf("\t\t...got it.\n");

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
DIR* turn_left()//char *dir, char *cwd, int cwdsize)
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

    return 0;// turn(dir, (char *)"L", cwd, cwdsize);
}

CPG_SECTION
DIR* turn_right()//char *dir, char *cwd, int cwdsize)
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

    return 0;// turn(dir, (char *)"R", cwd, cwdsize);
}

SENSORIMOTOR_SECTION
int sense_loc_feat(char *loc_dir_name)
{
    DIR *obj_feats_dirp=NULL;
    struct dirent *de_feats=NULL;

    // move sensor to a new location.
    obj_feats_dirp = opendir(loc_dir_name);
    // just make a random motor command for now.
    if (rand()/(float)RAND_MAX < 0.5) {
        turn_left();
    } else {
        turn_right();
    }
    if (chdir(loc_dir_name)<0) {
        perror("chdir() to location");
        return 0;
    }
    // acquire the sensory pattern.
    while ((de_feats = readdir(obj_feats_dirp)) != NULL) {
        if (de_feats->d_type == DT_REG) {
            printf("\t\tfeature: %s\n", de_feats->d_name);
            acquire_pattern(de_feats->d_name);
        }
    }
    closedir(obj_feats_dirp);
    return 1;
}

// explore the directory tree of rooms for files and directories.
int explore_world(char *dir_of_objs, unsigned int niter)
{
    char cwd[512];
    DIR *root_objs_dirp=NULL, *obj_locs_dirp=NULL;
    struct dirent *de_objs=NULL, *de_locs=NULL;

    // iterate through each object directory
    root_objs_dirp = opendir(dir_of_objs);
    while ((de_objs = readdir(root_objs_dirp)) != NULL) {
        for (unsigned int i=0; i<niter; i++) {
            if (!strcmp(de_objs->d_name, ".") ||
                !strcmp(de_objs->d_name, ".."))
                continue;
            printf("object: %s\n", de_objs->d_name);
            obj_locs_dirp = opendir(de_objs->d_name);
            if (chdir(de_objs->d_name) < 0)
                perror("chdir() to object");
            // iterate through each object n times
            memset(cwd, 0, sizeof(cwd));
            getcwd(cwd, sizeof(cwd));
            while ((de_locs = readdir(obj_locs_dirp)) != NULL) {
                if (!strcmp(de_locs->d_name, ".") ||
                    !strcmp(de_locs->d_name, ".."))
                    continue;
                printf("\tlocation: %s\n", de_locs->d_name);
                sense_loc_feat(de_locs->d_name);
                if (chdir(cwd) < 0)
                    perror("chdir");
            }
            if (chdir(dir_of_objs)<0)
                perror("chdir");
        }
        // signal next object
        raise(SIGUSR1);
    }

    /*
    if (scan_room(curdirp, dir, cwd, sizeof(cwd)) == 0)
        return 0;

    closedir(curdirp);
    if (explore_world(cwd) == 0)
        return 0;
    */

    return 1;
}

int main(int argc, char **argv)
{
    char *root=(char *)"/root/ace-core/examples/smi_world/";
    unsigned int num_iters=3;
    srand(time(NULL)+getpid());
    signal(SIGUSR1, reset_signal_callback);

    //printf("[*] Changing to root objects dir %s\n", root);
    if (chdir(root) < 0) {
        perror("chdir() failed");
        return 0;
    }
    if (!explore_world(root, num_iters))
        exit(-1);

    return 0;
}

