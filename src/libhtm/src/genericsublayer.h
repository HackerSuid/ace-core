#ifndef GENERICSUBLAYER_H_
#define GENERICSUBLAYER_H_

class GenericInput;
class SensoryRegion;

/*
 * base class for an htm sublayer and input region
 *
 * this is needed for two reasons:
 * 1. an HtmSublayer can't tell if its afferent connection originate from a
 *    codec or another sublayer, so a generic structure can represent
 *    either.
 * 2. both columns and codec units share many functions that can be
 *    deduplicated in a generic base class, GenericInput, which is contained
 *    within this generic base class.
 */
class GenericSublayer
{
protected:
    unsigned int height, width, depth;
    //std::vector <std::vector <GenericInput *> > input;
    GenericInput ***input;
    int numActiveInputs;
    SensoryRegion *motorPattern, *locationPattern;
    bool sensorimotorLayer;
public:
    GenericSublayer();
    ~GenericSublayer();
    bool IsSensorimotor() { return sensorimotorLayer; }
    unsigned int GetHeight();
    unsigned int GetWidth();
    unsigned int GetDepth();
    //std::vector <std::vector <GenericInput*> > GetInput();
    GenericInput*** GetInput();
    int GetNumActiveInputs()
    {
        return numActiveInputs;
    }
    SensoryRegion* GetMotorPattern() { return motorPattern; }
    SensoryRegion* GetLocationPattern()
    {
        return locationPattern;
    }
};

#endif

