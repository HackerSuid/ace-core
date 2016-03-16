#ifndef CELL_H_
#define CELL_H_

#include <vector>
#include "genericinput.h"

class HtmRegion;
class DendriteSegment;
class Synapse;

class Cell : public GenericInput
{
public:
    Cell();
    ~Cell();
    bool IsPredicted();
    bool WasPredicted();
    void SetPredicted(bool flag);
    bool IsLearning();
    bool WasLearning();
    void SetLearning(bool flag);
    DendriteSegment* NewSegment(HtmRegion *region, bool FirstPattern);
    DendriteSegment* GetMostActiveSegment();
    DendriteSegment* GetBestMatchingSegment(int lastActiveColumns);
    std::vector<DendriteSegment *> GetSegments();
    int GetNumSegments();
private:
    std::vector<DendriteSegment *> DistalDendriteSegments;
    int distalSegmentCount;
    bool predicted[2];
};

#endif

