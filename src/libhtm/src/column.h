#ifndef COLUMN_H_
#define COLUMN_H_

#include <vector>
#include "genericinput.h"

#define HIGH_TIER   0.90
#define MAX_ACTIVE_PERCENT  0.25

class GenericRegion;
class Cell;
class DendriteSegment;

struct ActivityLogEntry
{
    bool active;
    struct ActivityLogEntry *next;
};

class Column : public GenericInput
{
private:
    // 2 dimensional position within the cortical region.
    int x, y;
    // 2 dimensional position of columns natural center with the input space.
    int x_center, y_center;
    // parameters to implement columnar and synaptic boosting.
    int timeStep; // bookkeeping run-time window
    double boost, activeDutyCycle, overlapDutyCycle;
    int activityCycleWindow;
    struct ActivityLogEntry *activityLogHead, *activityLogTail;
    struct ActivityLogEntry *overlapLogHead, *overlapLogTail;
    // active columns within a local neighborhood radius and minimum number
    // of synapses required to fire for a column to become active.
    float localActivity, columnComplexity;
    int rec_field_sz, overlap;

    int numCells;
    std::vector<Cell *> cells;
    Column **neighbors;
    int numNeighbors;
    int inhibitionRadius;
    bool highTier;
    DendriteSegment *ProximalDendriteSegment;

    void UpdateBoostingStructure(
        struct ActivityLogEntry **head,
        struct ActivityLogEntry **tail,
        unsigned int flag
    );
public:
    Column(
        unsigned int numCells,
        int rfsz,
        float localActivity,
        float columnComplexity,
        bool highTier,
        int activityCycleWindow
    );
    ~Column();
    void InitializeProximalDendrite(
        int x, int y,
        GenericRegion *lower, 
        int x_ratio, int y_ratio
    );
    void RefreshNewPattern(GenericRegion *NewPattern);

    // Spatial Pooler functions
    void ComputeOverlap();
    void UpdateActivationBoost(double minActivityLevel);
    void UpdateOverlapBoost(double minActivityLevel);
    double ComputeActivationAvg(bool recompute);
    double ComputeOverlapAvg(bool recompute);
    void ModifySynapses();
    void NextTimestep();
    void BoostPermanences();

    // Sequence Memory functions
    Cell* GetBestMatchingCell(
        DendriteSegment **segment,
        int numActiveColumns,
        bool FirstPattern
    );

    // accessors
    bool IsHighTierColumn();
    int GetX();
    int GetY();
    int GetCenterX();
    int GetCenterY();
    int GetRecFieldSz();
    int GetMinOverlap();
    void SetOverlap(int newOverlap, bool Boosting);
    void SetActive(bool flag, bool Boosting);
    int GetOverlap();
    double GetBoost();
    double GetMinActivityLevel();
    DendriteSegment* GetProximalDendriteSegment();
    Column** GetNeighbors(
        Column ***columns,
        int radius,
        int xmax,
        int ymax
    );
    int GetNumNeighbors();
    int GetLocalActivity();

    int GetNumCells();
    std::vector<Cell *> GetCells();
};

#endif

