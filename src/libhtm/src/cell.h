#ifndef CELL_H_
#define CELL_H_

#include <vector>
#include "genericinput.h"

typedef enum {
    SENSORY_DISTAL,
    MOTOR_DISTAL,
    LATERAL_DISTAL
} input_t;

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
    DendriteSegment* NewSegment(HtmSublayer *sublayer, bool FirstPattern);
    bool AddSynapsesFromSublayer(
        HtmSublayer *thisSublayer,
        GenericSublayer *src,
        input_t inType,
        DendriteSegment *seg
    );
    void RemoveSegment(int segidx);
    DendriteSegment* GetMostActiveSegment();
    DendriteSegment* GetBestMatchingSegment(
        int *bestSegIdx,
        HtmSublayer *sublayer
    );
    std::vector<DendriteSegment *> GetSegments();
    unsigned int GetNumSegments();
    Column* GetParentColumn() { return parentColumn; }
private:
    Column *parentColumn;
    std::vector<DendriteSegment *> DistalDendriteSegments;
    int distalSegmentCount;
    bool predicted[2];
};

#endif

