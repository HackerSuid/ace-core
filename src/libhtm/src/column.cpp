#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "column.h"
#include "genericregion.h"
#include "util.h"
#include "dendritesegment.h"
#include "synapse.h"
#include "cell.h"

Column::Column(
    unsigned int numCells,
    int rfsz,
    float localActivity,
    float columnComplexity,
    bool highTier,
    int activityCycleWindow)
{
    ProximalDendriteSegment = NULL;
    rec_field_sz = rfsz;
    overlap = 0;
    boost = 1.0;
    neighbors = NULL;
    inhibitionRadius = 0;
    numNeighbors = 0;
    this->localActivity = localActivity;
    this->columnComplexity = columnComplexity;
    this->highTier = highTier;
    activeDutyCycle = overlapDutyCycle = -1.0;
    this->activityCycleWindow = activityCycleWindow;
    activityLogHead = activityLogTail = NULL;
    overlapLogHead = overlapLogTail = NULL;
    timeStep = 0;

    this->numCells = numCells;
    for (unsigned int i=0; i<numCells; i++)
        cells.push_back(new Cell());
}

Column::~Column()
{
}

/*
 * Using a point (x,y) as the center of the column's receptive field, form synapses
 * on the shared proximal dendrite segment to a random subset of inputs from the
 * lower level beneath the one containing this column. Depending on what level
 * of the cortical pathway the region/layer this column is a member of, the
 * receptive field may consist of other columns or sensory input (codec). The
 * x and y ratios are used to compute the center position that the column has
 * with its input.
 *
 * For example, the afferent pathway of a biological region may originate from
 * the lateral geniculate nuclei (LGN) of the thalamus or from another cortical
 * region.
 */
void Column::InitializeProximalDendrite(
    int x, int y,
    GenericRegion *lower,
    int x_ratio, int y_ratio)
{
    GenericInput ***inputs = lower->GetInput();
    unsigned int h = lower->GetHeight();
    unsigned int w = lower->GetWidth();
    int syn_pos[rec_field_sz][2], a, b, x_idx, y_idx;

    this->x = x;
    this->y = y;
    // compute the natural center of the column to the input.
    x_center = this->x*x_ratio+(x_ratio/2);
    y_center = this->y*y_ratio+(y_ratio/2);

    memset(syn_pos, 0, sizeof(syn_pos));
    ProximalDendriteSegment = new DendriteSegment;
    int rec_fld_rad = rec_field_sz/4;
    for (int i=0; i<rec_field_sz; i++) {
        do {
            int rnd_rad_x = rand()%rec_fld_rad;
            a = rand()<(RAND_MAX/2)? rnd_rad_x : -1*rnd_rad_x;
            int rnd_rad_y = rand()%rec_fld_rad;
            b = rand()<(RAND_MAX/2)? rnd_rad_y : -1*rnd_rad_y;
            // these may exceed the input boundaries. be sure to limit them.
            x_idx = x_center+a;
            x_idx = x_idx > 0 ? (x_idx>=(int)w? w-1 : x_idx) : 0;
            y_idx = y_center+b;
            y_idx = y_idx > 0 ? (y_idx>=(int)h? h-1 : y_idx) : 0;
        } while (!UniqueIdx(x_idx, y_idx, (int *)syn_pos, i));
        syn_pos[i][0] = x_idx;
        syn_pos[i][1] = y_idx;
        ProximalDendriteSegment->NewSynapse(new Synapse(inputs[x_idx][y_idx], x_idx, y_idx));
    }
}

void Column::RefreshNewPattern(GenericRegion *NewPattern)
{
    ProximalDendriteSegment->RefreshSynapses(NewPattern);
}

void Column::ComputeOverlap()
{
    overlap = 0;
    std::vector<Synapse *> synapses = ProximalDendriteSegment->GetSynapses();
    for (int i=0; i<rec_field_sz; i++)
        if (synapses[i]->IsFiring())
            overlap++;
}

void Column::ModifySynapses()
{
    std::vector<Synapse *> synapses = ProximalDendriteSegment->GetSynapses();
    for (int i=0; i<rec_field_sz; i++)
        if (synapses[i]->GetSource()->IsActive())
            synapses[i]->IncPerm();
        else
            synapses[i]->DecPerm();
}

// accessors
bool Column::IsHighTierColumn()
{
    if (!highTier)
        return false;
    if (overlap < HIGH_TIER * rec_field_sz)
        return false;
    return true;
}

int Column::GetX()
{
    return x;
}

int Column::GetY()
{
    return y;
}

int Column::GetCenterX()
{
    return x_center;
}

int Column::GetCenterY()
{
    return y_center;
}

int Column::GetRecFieldSz()
{
    return rec_field_sz;
}

int Column::GetMinOverlap()
{
    return rec_field_sz * columnComplexity;
}

int Column::GetOverlap()
{
    return overlap;
}

void Column::SetOverlap(int newOverlap, bool Boosting)
{
    overlap = newOverlap;
    if (Boosting)
        UpdateBoostingStructure(&overlapLogHead, &overlapLogTail, newOverlap? 1 : 0);
}

void Column::SetActive(bool flag, bool Boosting)
{
    active[1] = active[0];
    active[0] = flag;

    if (Boosting)
        UpdateBoostingStructure(&activityLogHead, &activityLogTail, flag);
}

/*
 * New entries pile onto the tail node. Old entries fall off the head.
 */
void Column::UpdateBoostingStructure(
    struct ActivityLogEntry **head,
    struct ActivityLogEntry **tail,
    unsigned int flag)
{
    struct ActivityLogEntry *newEntry = (struct ActivityLogEntry *)malloc(sizeof(struct ActivityLogEntry));
    newEntry->active = flag;
    newEntry->next = NULL;

    if (timeStep < activityCycleWindow) {
        // update the activity list to add a new entry.
        if (timeStep == 0) {
            *head = newEntry;
            *tail = newEntry;
        } else {
            (*tail)->next = newEntry;
            *tail = newEntry;
        }
    } else {
        // update the activity list to replace oldest entry.
        (*tail)->next = newEntry;
        *tail = newEntry;
        struct ActivityLogEntry *oldHead = *head;
        *head = (*head)->next;
        delete oldHead;
    }
}

/*
 * boost increases linearly as the columns activity level falls below the
 * minimum. the minimum is determined as a function of the maximum activity
 * of the neighboring columns.
 */
void Column::UpdateActivationBoost(double minActivityLevel)
{
    double currActivity = ComputeActivationAvg(true);

    if ((currActivity < minActivityLevel) && timeStep >= (activityCycleWindow/2))
        boost++;
    else
        boost = 1.0;
}

void Column::UpdateOverlapBoost(double minActivityLevel)
{
    double currActivity = ComputeOverlapAvg(true);

    if ((currActivity < minActivityLevel) && timeStep >= (activityCycleWindow/2))
        BoostPermanences();
}

double Column::ComputeActivationAvg(bool recompute)
{
    struct ActivityLogEntry *curr=activityLogHead;
    double avg=0;

    if (!recompute && activeDutyCycle >= 0)
        return activeDutyCycle;

    do {
        avg += curr->active;
    } while ((curr = curr->next));
    activeDutyCycle = avg/activityCycleWindow;

    return activeDutyCycle;
}

double Column::ComputeOverlapAvg(bool recompute)
{
    struct ActivityLogEntry *curr=overlapLogHead;
    double avg=0;

    if (!recompute && overlapDutyCycle >= 0)
        return overlapDutyCycle;

    do {
        avg += curr->active;
    } while ((curr = curr->next));
    overlapDutyCycle = avg/activityCycleWindow;

    return overlapDutyCycle;
}

double Column::GetMinActivityLevel()
{
    double maxActivityLevel = -1;
    int numNeighbors = GetNumNeighbors();

    for (int i=0; i<numNeighbors; i++) {
        double neighborActivity = neighbors[i]->ComputeActivationAvg(false);
        if (neighborActivity > maxActivityLevel)
            maxActivityLevel = neighborActivity;
    }

    return maxActivityLevel*MAX_ACTIVE_PERCENT;
}

double Column::GetBoost()
{
    return boost;
}

void Column::BoostPermanences()
{
    std::vector<Synapse *> synapses = ProximalDendriteSegment->GetSynapses();
    for (int i=0; i<rec_field_sz; i++)
        synapses[i]->IncPerm(0.1);
}

void Column::NextTimestep()
{
    timeStep++;
}

DendriteSegment* Column::GetProximalDendriteSegment()
{
    return ProximalDendriteSegment;
}

Column** Column::GetNeighbors(
    Column ***columns,
    int radius,
    int xmax,
    int ymax)
{
    // same radius will return the same neighbors.
    if (radius == inhibitionRadius)
        return neighbors;
    if (neighbors)
        free(neighbors);
    
    int left   = x-radius > 0 ? x-radius : 0;
    int right  = x+radius < xmax ? x+radius : xmax-1;
    int top    = y-radius > 0 ? y-radius : 0;
    int bottom = y+radius < ymax ? y+radius : ymax-1;
    unsigned int n = 0;

    neighbors = (Column **)malloc(sizeof(Column *)*(right-left+1)*(bottom-top+1));
    for (int i=left; i<=right; i++)
        for (int j=top; j<=bottom; j++)
            if ((i != x) || (j != y))
                neighbors[n++] = columns[i][j];

    numNeighbors = n;

    return neighbors;
}

int Column::GetNumNeighbors()
{
    return numNeighbors;
}

int Column::GetLocalActivity()
{
    return numNeighbors * localActivity;
}

/*
 * Return the cell with the best matching segment. If no cells have a good
 * enough match, then return the one with the fewest segments. If none of them
 * have any segments, then return the first one.
 */
Cell* Column::GetBestMatchingCell(
    DendriteSegment **segment,
    int numActiveColumns,
    bool FirstPattern)
{
    if (FirstPattern) {
//        printf("\tfirst pattern...\n");
        return cells[0];
    }

    Cell *BestCell = NULL;
    int bestCellIdx = -1;
    DendriteSegment *bestSeg = NULL;
    int highestNumSyns = -1;

    Cell *fewestSegCell = NULL;
    int cellNumSegments, fewestSegs = -1, fewestSegCellIdx = -1;

    // segment will be NULL if no segments are found.
    *segment = bestSeg;
    for (int i=0; i<numCells; i++) {
        // track the cell with the fewest number of segments.
        cellNumSegments = cells[i]->GetNumSegments();
        if ((fewestSegs < 0) || (fewestSegs >= 0 && cellNumSegments < fewestSegs)) {
            fewestSegs = cellNumSegments;
            fewestSegCell = cells[i];
            fewestSegCellIdx = i;
        }

        // for cell i, get the segment with the most active synapses in the
        // previous timestep.
        bestSeg = cells[i]->GetBestMatchingSegment(numActiveColumns);
//        if (i>0 && cellNumSegments>0 && bestSeg)
//            printf("\tchecking if best seg (out of %d) [0x%08x] on c%d is best...", cellNumSegments, bestSeg, i);
        // if a segment is returned, see if it had more synapses that could
        // have predicted this activity than the highest so far.
        if (bestSeg) {
            if (bestSeg->GetNumIsNearActiveSynapses() > highestNumSyns) {
//                printf("\tcell %d has the most (near) active synapses [%d] on segment 0x%08x\n",
//                    i, bestSeg->GetNumIsNearActiveSynapses(), (unsigned int)bestSeg);
//                std::vector<Synapse *> theSyns = bestSeg->GetSynapses();
//                printf("\t\t");
//                for (int x=0; x<theSyns.size(); x++)
//                    printf("%f ", theSyns[x]->GetPerm());
//                printf("\n");
//                if (i>0 && cellNumSegments>0)
//                    printf("yep.\n");
                highestNumSyns = bestSeg->GetNumIsActiveSynapses();
                BestCell = cells[i];
                *segment = bestSeg;
                bestCellIdx = i;
            } else {
//                if (i>0 && cellNumSegments>0)
//                    printf("nope.\n");
            }
        }
    }

    if (!BestCell) {
        BestCell = fewestSegCell;
        bestCellIdx = fewestSegCellIdx;
    }

//    printf("\t[0x%08x] cell %d chosen for learning\n",
//        (unsigned int)this, bestCellIdx);

    return BestCell;
}

int Column::GetNumCells()
{
    return numCells;
}

std::vector<Cell *> Column::GetCells()
{
    return cells;
}
