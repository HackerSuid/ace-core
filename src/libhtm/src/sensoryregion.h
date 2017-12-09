#ifndef SENSORY_H_
#define SENSORY_H_

#include "genericsublayer.h"

class SensoryInput;

class SensoryRegion : public GenericSublayer
{
private:
    SensoryRegion *next;
    bool reset;
public:
    SensoryRegion(
        SensoryInput ***bits,
        int numActiveInputs,
        unsigned int w,
        unsigned int h,
        unsigned int d,
        SensoryRegion *mp,
        bool resetFlag=false
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
    void SetLocationPattern(SensoryRegion *lp)
    {
        locationPattern = lp;
    }
    bool GetReset() { return reset; }
};

#endif

