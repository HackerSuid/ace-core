#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "htmregion.h"
#include "sensoryregion.h"
#include "sensoryinput.h"
#include "codec.h"

BitmapCodec::BitmapCodec()
    : SensoryCodec()
{
}

BitmapCodec::~BitmapCodec()
{
}

// Take a bitmap file and convert to an SensoryRegion
SensoryRegion* BitmapCodec::GetPattern(int sense_fd, bool Learning)
{
    BITMAPHEADER *bmh;
    Pixel rgb;
    SensoryRegion *pattern=NULL;
    SensoryInput ***input=NULL;
    int numActiveInputs = 0;

    if ((bmh = ReadBitmapHeader(sense_fd)) == NULL)
        return NULL;

    // read each pixel value and map to a bit vector
    //   black => 0x00 (active)
    //   white => 0xFF (inactive)
    input = (SensoryInput ***)malloc(sizeof(SensoryInput **) * bmh->bmih.height);
    for (int i=0; i<bmh->bmih.height; i++) {
        // flip the image to display correctly on the unit grid.
        input[(bmh->bmih.height-1)-i] = (SensoryInput **)malloc(sizeof(SensoryInput *) * bmh->bmih.width);
        for (int j=0; j<bmh->bmih.width; j++) {
            read(sense_fd, &rgb, sizeof(rgb));
            input[(bmh->bmih.height-1)-i][j] = new SensoryInput(j, i);
            input[(bmh->bmih.height-1)-i][j]->SetActive(!(rgb.r|rgb.g|rgb.b));
            if (!(rgb.r|rgb.g|rgb.b))
                numActiveInputs++;
        }
    }
    //fclose(fp);

    // input patterns will have zero cells.
    pattern = new SensoryRegion(
        input, numActiveInputs,
        bmh->bmih.width, bmh->bmih.height,
        0, NULL
    );

    return pattern;
}

BITMAPHEADER* BitmapCodec::ReadBitmapHeader(int fd)
{
    BITMAPHEADER *bmh = (BITMAPHEADER *)malloc(sizeof(BITMAPHEADER));

    //if ((*fp = fopen(filename, "r")) == NULL) {
    //    fprintf(stderr, "%s: %s\n", filename, strerror(errno));
    //    return NULL;
    //}

    read(fd, (void *)&(bmh->bmfh), sizeof(BITMAPFILEHEADER));
    read(fd, (void *)&(bmh->bmih), sizeof(BITMAPINFOHEADER));
    //fread((BITMAPFILEHEADER *)&(bmh->bmfh), sizeof(BITMAPFILEHEADER), 1, *fp);
    //fread((BITMAPINFOHEADER *)&(bmh->bmih), sizeof(BITMAPINFOHEADER), 1, *fp);

    return bmh;
}

