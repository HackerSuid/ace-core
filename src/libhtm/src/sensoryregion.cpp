#include <string.h>
#include <stdlib.h>

#include "sensoryregion.h"
#include "sensoryinput.h"

SensoryRegion::SensoryRegion(
    char *filename,
    SensoryInput ***bits,
    unsigned int w,
    unsigned int h,
    unsigned int d)
{
    this->filename = strdup(filename);
    this->width = w;
    this->height = h;
    this->depth = d;

    // initialize the input bits
    this->input = (GenericInput ***)malloc(sizeof(GenericInput **) * h);
    for (unsigned int i=0; i<h; i++) {
        input[i] = (GenericInput **)malloc(sizeof(GenericInput *) * w);
        for (unsigned int j=0; j<w; j++)
            input[i][j] = bits[i][j];
    }       
}

SensoryRegion::~SensoryRegion()
{
    delete this->filename;
}

char* SensoryRegion::GetFilename()
{
    return this->filename;
}

SensoryInput*** SensoryRegion::GetInput()
{
    return (SensoryInput ***)input;
}

SensoryRegion* SensoryRegion::GetNext()
{
    return next;
}

void SensoryRegion::SetNext(SensoryRegion *pattern)
{
    next = pattern;
}

