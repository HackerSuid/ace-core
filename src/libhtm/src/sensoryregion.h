#ifndef SENSORY_H_
#define SENSORY_H_

#include "genericsublayer.h"

class SensoryInput;

class SensoryRegion : public GenericSublayer
{
private:
    SensoryRegion *next;
public:
    SensoryRegion(
        SensoryInput ***bits,
        int numActiveInputs,
        unsigned int w,
        unsigned int h,
        unsigned int d,
        SensoryRegion *mp
    );
    ~SensoryRegion();
    SensoryInput*** GetInput()
    {
        return (SensoryInput ***)input;
    }
    SensoryRegion* GetNext()
    {
        return next;
    }
    void SetNext(SensoryRegion *pattern)
    {
        next = pattern;
    }
    void SetMotorPattern(SensoryRegion *mp)
    {
        motorPattern = mp;
    }
};

#endif

