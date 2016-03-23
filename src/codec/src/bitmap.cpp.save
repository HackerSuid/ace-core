#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

#include "bitmap.h"
#include "htmregion.h"
#include "sensoryregion.h"
#include "sensoryinput.h"

// Register with the base Codec class.
bool const BitmapCodec::registered = Codec::Register<BitmapCodec>();

/*
__attribute__((constructor)) void LoadBitmapCodec(void) {
    printf("bitmap codec loaded!\n");
}
*/

BitmapCodec::BitmapCodec()
    : Codec()
{
    pidx = 0;
    codecName = strdup("Bitmap");
}

BitmapCodec::~BitmapCodec()
{
    if (TargetPath) free(TargetPath);
    if (codecName) free(codecName);
}

bool BitmapCodec::Init(char *target_path)
{
    DIR *dp;

    // check that directory exists.
    if ((dp = opendir(target_path)) == NULL) {
        fprintf(stderr, "%s error: failed to open %s.\n", __func__, target_path);
        return false;
    }
    closedir(dp);

    this->TargetPath = strdup(target_path);

    return true;
}

// Take a bitmap file and convert to an SensoryRegion
SensoryRegion* BitmapCodec::GetPattern()
{
    BITMAPHEADER *bmh;
    FILE *fp = NULL;
    char filename[128];
    Pixel rgb;
    SensoryRegion *pattern=NULL;
    SensoryInput ***input=NULL;

    snprintf(filename, sizeof(filename), "%s/%d.bmp", TargetPath, pidx++);
    if ((bmh = ReadBitmapHeader(filename, &fp)) == NULL)
        return NULL;

    // read each pixel value and map to a bit vector
    //   black => 0x00 (active)
    //   white => 0xFF (inactive)
    input = (SensoryInput ***)malloc(sizeof(SensoryInput **) * bmh->bmih.height);
    for (int i=0; i<bmh->bmih.height; i++) {
        // flip the image to display correctly on the unit grid.
        input[(bmh->bmih.height-1)-i] = (SensoryInput **)malloc(sizeof(SensoryInput *) * bmh->bmih.width);
        for (int j=0; j<bmh->bmih.width; j++) {
            fread(&rgb, sizeof(rgb), 1, fp);
            input[(bmh->bmih.height-1)-i][j] = new SensoryInput;
            input[(bmh->bmih.height-1)-i][j]->SetActive(!(rgb.r|rgb.g|rgb.b));
        }
    }
    fclose(fp);

    // input patterns will have zero cells.
    pattern = new SensoryRegion(filename, input, bmh->bmih.width, bmh->bmih.height, 0);

    return pattern;
}

// whether or not the codec is processing the first pattern of a pre-determined
// sequence.
bool BitmapCodec::FirstPattern(SensoryRegion *InputPattern)
{
    char *filename = InputPattern->GetFilename();

    if (strchr(filename, '/'))
        filename = strrchr(filename, '/')+1;

    char *ext = strrchr(filename, '.');
    ext = 0;

    int patt_idx = atoi(filename);
    
    return patt_idx? false : true;
}

void BitmapCodec::Reset()
{
    pidx = 0;
}

BITMAPHEADER* BitmapCodec::ReadBitmapHeader(const char *filename, FILE **fp)
{
    BITMAPHEADER *bmh = (BITMAPHEADER *)malloc(sizeof(BITMAPHEADER));

    if ((*fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "%s: %s\n", filename, strerror(errno));
        return NULL;
    }

    fread((BITMAPFILEHEADER *)&(bmh->bmfh), sizeof(BITMAPFILEHEADER), 1, *fp);
    fread((BITMAPINFOHEADER *)&(bmh->bmih), sizeof(BITMAPINFOHEADER), 1, *fp);

    return bmh;
}

