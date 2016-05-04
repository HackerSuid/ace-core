#ifndef HTMSUBLAYER_H_
#define HTMSUBLAYER_H_

#include <vector>

#include "genericsublayer.h"
#include <list>

#define PRED_STAB_WINDOW_SZ 30

class SensoryRegion;
class SensoryInput;
class Htm;
class HtmSublayer;
class Column;
class Cell;
class Synapse;
class DendriteSegment;

class SegmentUpdate
{
public:
    SegmentUpdate(Cell *cell, DendriteSegment *segment, bool connected)
    {
        this->cell = cell;
        this->segment = segment;
        this->connected = connected;
    }
    ~SegmentUpdate() {}
    Cell* GetCell() { return cell; }
    DendriteSegment* GetSegment() { return segment; }
    bool IsConnected() { return connected; }
private:
    Cell *cell;
    DendriteSegment *segment;
    bool connected;
};

class HtmSublayer : public GenericSublayer
{
private:
    GenericSublayer *lower, *higher;
    HtmSublayer *sublayers;
    int rec_field_sz, inhibitionRadius;
    float localActivity, columnComplexity;
    std::vector<SegmentUpdate *> segmentUpdateList;
    int numActiveColumns[2];
    Htm *htmPtr;
    std::list<float> predStabWindow;

    bool _EligibleToFire(Column *col);
    void _ComputeInhibitionRadius(Column ***columns);
    void _EnqueueSegmentUpdate(
        Cell *cell,
        DendriteSegment *segment,
        bool connected
    );
    void _DequeueSegmentUpdate(SegmentUpdate *segUpdate, bool reinforce);
public:
    HtmSublayer(
        unsigned int h,
        unsigned int w,
        unsigned int cpc,
        Htm *htmPtr
    );
    ~HtmSublayer();
    void AllocateColumns(
        float rfsz,
        float localActivity,
        float columnComplexity,
        bool highTier,
        int activityCycleWindow
    );
    void InitializeProximalDendrites();
    void RefreshLowerSynapses();
    void CLA(bool Learning, bool allowBoosting);
    void SpatialPooler(bool Learning, bool allowBoosting);
    void SequenceMemory(bool Learning, bool firstPattern);
    void NewTimestep();
    int LastActiveColumns();
    int CurrentActiveColumns();
    // accessors
    Column*** GetInput();
    void setlower(GenericSublayer *reg);
    void sethigher(GenericSublayer *reg);
    GenericSublayer* GetLower() { return this->lower; }
    GenericSublayer* GetHigher() { return this->higher; }
    std::list<float> GetPredictionStabilityWindow() { return predStabWindow; }
};

#endif
