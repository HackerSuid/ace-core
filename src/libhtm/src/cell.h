#ifndef CELL_H_
#define CELL_H_

#include <vector>
#include "genericinput.h"
#include "dendritesegment.h"

class GenericSublayer;
class HtmSublayer;
class Column;
class DendriteSegment;
class DistalDendrite;
class Synapse;

class Cell : public GenericInput
{
public:
    Cell(Column *col, unsigned int idx);
    ~Cell();
    bool IsPredicted();
    bool WasPredicted();
    void SetPredicted(bool flag);
    DistalDendrite* NewSegment(HtmSublayer *sublayer, bool FirstPattern);
    bool AddSynapsesFromSublayer(
        HtmSublayer *thisSublayer,
        GenericSublayer *src,
        input_t inType,
        DistalDendrite *seg
    );
    void RefreshDendrites(GenericSublayer *NewPattern);
    void RemoveSegment(int segidx);
    DistalDendrite* GetMostActiveSegment();
    DistalDendrite* GetBestMatchingSegment(
        int *bestSegIdx,
        HtmSublayer *sublayer
    );
    std::vector<DistalDendrite *> GetSegments()
    {
        return DistalDendriteSegments;
    }
    unsigned int GetNumSegments()
    {
        return DistalDendriteSegments.size();
    }
    Column* GetParentColumn() { return parentColumn; }
    unsigned int GetColIdx() { return colIdx; }
private:
    Column *parentColumn;
    unsigned int colIdx;
    std::vector<DistalDendrite *> DistalDendriteSegments;
    int distalSegmentCount;
    bool predicted[2];
};

#endif

