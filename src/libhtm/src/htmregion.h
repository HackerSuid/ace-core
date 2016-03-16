#ifndef HTMREGION_H_
#define HTMREGION_H_

#include <vector>

#include "genericregion.h"

class SensoryRegion;
class SensoryInput;
class Htm;
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

class HtmRegion : public GenericRegion
{
private:
    GenericRegion *lower, *higher;
    int rec_field_sz, inhibitionRadius;
    float localActivity, columnComplexity;
    std::vector<SegmentUpdate *> segmentUpdateList;
    //SegmentUpdate **segmentUpdateList;
    int numActiveColumns[2];
    Htm *htmPtr;

    bool _EligibleToFire(Column *col);
    void _ComputeInhibitionRadius(Column ***columns);
    void _EnqueueSegmentUpdate(
        Cell *cell,
        DendriteSegment *segment,
        bool connected
    );
    void _DequeueSegmentUpdate(SegmentUpdate *segUpdate, bool reinforce);
public:
    HtmRegion(
        unsigned int h,
        unsigned int w,
        unsigned int cpc,
        Htm *htmPtr
    );
    ~HtmRegion();
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
    void setlower(GenericRegion *reg);
    void sethigher(GenericRegion *reg);
    GenericRegion* GetLower() { return this->lower; }
    GenericRegion* GetHigher() { return this->higher; }
};

#endif

