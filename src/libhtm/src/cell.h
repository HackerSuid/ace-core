#ifndef CELL_H_
#define CELL_H_

#include <vector>
#include "genericinput.h"

class GenericSublayer;
class HtmSublayer;
class Column;
class DendriteSegment;
class Synapse;

class Cell : public GenericInput
{
public:
    Cell(Column *col);
    ~Cell();
    bool IsPredicted();
    bool WasPredicted();
    void SetPredicted(bool flag);
    bool IsLearning();
    bool WasLearning();
    void SetLearning(bool flag);
    DendriteSegment* NewSegment(HtmSublayer *sublayer, bool FirstPattern);
    bool AddSynapsesFromSublayer(
        HtmSublayer *thisSublayer,
        GenericSublayer *src,
        DendriteSegment *seg
    );
    void RemoveSegment(int segidx);
    DendriteSegment* GetMostActiveSegment();
    DendriteSegment* GetBestMatchingSegment(
        int *bestSegIdx,
        int lastActiveColumns
    );
    std::vector<DendriteSegment *> GetSegments();
    int GetNumSegments();
    Column* GetParentColumn() { return parentColumn; }
private:
    Column *parentColumn;
    std::vector<DendriteSegment *> DistalDendriteSegments;
    int distalSegmentCount;
    bool predicted[2];
};

#endif

