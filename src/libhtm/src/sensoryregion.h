#ifndef SENSORY_H_
#define SENSORY_H_

#include "genericsublayer.h"

class SensoryInput;

class SensoryRegion : public GenericSublayer
{
private:
    SensoryRegion *next;
public:
    SensoryRegion(SensoryInput ***bits, unsigned int w, unsigned int h, unsigned int d);
    ~SensoryRegion();
    SensoryInput*** GetInput();
    SensoryRegion* GetNext();
    void SetNext(SensoryRegion *pattern);
};

#endif

