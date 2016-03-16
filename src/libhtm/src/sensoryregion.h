#ifndef SENSORY_H_
#define SENSORY_H_

#include "genericregion.h"

class SensoryInput;

class SensoryRegion : public GenericRegion
{
private:
    char *filename;
    SensoryRegion *next;
public:
    SensoryRegion(char *filename, SensoryInput ***bits, unsigned int w, unsigned int h, unsigned int d);
    ~SensoryRegion();
    char* GetFilename();
    SensoryInput*** GetInput();
    SensoryRegion* GetNext();
    void SetNext(SensoryRegion *pattern);
};

#endif

