#include <cstddef>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include "genericsublayer.h"
#include "htmsublayer.h"
#include "sensoryregion.h"
#include "htm.h"
#include "column.h"
#include "cell.h"
#include "dendritesegment.h"
#include "synapse.h"

Cell::Cell(Column *col)
{
    parentColumn = col;
    memset(&predicted[0], false, sizeof(bool)*2);
}

Cell::~Cell()
{
}

void Cell::SetPredicted(bool flag)
{
    predicted[1] = predicted[0];
    predicted[0] = flag;
}

bool Cell::IsPredicted()
{
    return predicted[0];
}

bool Cell::WasPredicted()
{
    return predicted[1];
}

/*
 * Forms a new distal dendrite segment that contains synapses
 * for sensory pattern prediction that depends on what kind of
 * context the pattern is in.
 *
 * 1. For cells in L2/3, distal segments only synapse with
 *    other L2/3 cells, so synapses are only created with
 *    nearby cells in the same sublayer (although, in real
 *    brains, axons from L2/3 of the contralateral cortical
 *    hemisphere are also used).
 * 2. For cells in L4, distal dendrite segments synapse with
      two sources of input. Lateral connections are formed
      like in L2/3. Additionally, synapses are formed with
      input bits from the motor pattern.

search through all the cells within the sublayer for cells that were
 * previously active and learning a pattern in context, and create a new
 * segment with connections to a subset of these cells.
 *
 * biologically, each cell only receives axons from other cells nearby,
 * not globally
 */
DendriteSegment* Cell::NewSegment(HtmSublayer *sublayer, bool FirstPattern)
{
    DendriteSegment *newSeg = new DendriteSegment(sublayer->IsSensorimotor());
    bool synsFound = false;

    /*
     * For sensorimotor sublayers, the first pattern will only
     * be in motor-context. For purely sensory sublayers, the
     * pattern will have no context.
     */

    if (!FirstPattern) {
        //printf("\tAdding synapses to distal dendrite from sensory pattern.\n");
        synsFound = AddSynapsesFromSublayer(sublayer, sublayer, false, newSeg);
    }

    if (sublayer->IsSensorimotor()) {
        SensoryRegion *mp = sublayer->GetLower()->GetMotorPattern();
        //sublayer->GetHtmPtr()->PrintPattern(mp);
        if (mp) {
            //printf("\tAdding synapses to distal dendrite from motor pattern.\n");
            synsFound = AddSynapsesFromSublayer(
                sublayer, (GenericSublayer *)mp, true, newSeg
            );
            //newSeg->SetNoTemporalContext();
            //DistalDendriteSegments.push_back(newSeg);
            //return NULL;
            DistalDendriteSegments.push_back(newSeg);
            return newSeg;
        } else {
            printf("\tNo motor pattern found.\n");
            newSeg->SetNoTemporalContext();
            DistalDendriteSegments.push_back(newSeg);
            return NULL;
        }
    } else {
        printf("\tSublayer is not sensorimotor-capable.\n");
        newSeg->SetNoTemporalContext();
        DistalDendriteSegments.push_back(newSeg);
        return NULL;
    }

    // if zero synapses were formed, then there was no previous activity
    // in the sequence so skip adding a new segment.
    if (synsFound) {
        //printf("\t\tfound %d cells\n", synsFound);
        DistalDendriteSegments.push_back(newSeg);
    } else {
        printf("\t\tDid not find any previous activity.\n");
        delete newSeg;
        newSeg = NULL;
    }


    return newSeg;
}

bool Cell::AddSynapsesFromSublayer(
    HtmSublayer *thisSublayer,
    GenericSublayer *src,
    bool motorSrc,
    DendriteSegment *seg)
{
    GenericInput ***inputBits = src->GetInput();

    int w = src->GetWidth();
    int h = src->GetHeight();
    int d = src->GetDepth();

    int cellx = parentColumn->GetX();
    int celly = parentColumn->GetY();

    bool foundSampleSize = false;
    int synsFound = 0;
    int subsampleSz =
        (motorSrc ? thisSublayer->CurrentActiveColumns() :
                    thisSublayer->LastActiveColumns()) *
        DendriteSegment::GetSubsamplePercent();
    std::vector<GenericInput *> projections;

    auto il = { h-1-celly, celly, w-1-cellx, cellx };

    //printf("\tcreating %d synapses for cell in column (%d,%d)\n",
            //subsampleSz, cellx, celly);
    //printf("subsampleSz = %d * %f = %d\n",
    //        thisSublayer->CurrentActiveColumns(),
    //        DendriteSegment::GetSubsamplePercent(), subsampleSz
    //);
    for (int distance=1; distance<std::max(il); distance++) {
        for (int yd=-distance; yd<=distance&&!foundSampleSize; yd++) {
            for (int xd=-distance; xd<=distance&&!foundSampleSize; xd++) {
                if (abs(xd) != distance && abs(yd) != distance)
                    continue;
                int x = cellx+xd;
                int y = celly+yd;
                // minimum limitations
                if ((x<0 || y<0) || (x==cellx && y==celly))
                    continue;
                // maximum limitations
                if ((x>=w) || (y>=h))
                    continue;
                /*
                 * depth indicates if inputBits are from a SensoryRegion or
                 * HtmSublayer.
                 *
                 * Synapses from HtmSublayer to SensoryRegion connect to
                 * inputBits[y][x].
                 *
                 * Synapses from HtmSublayer to HtmSublayer connect to the
                 * learning cell in inputBits[y][x].
                 */
                projections.clear();
                if (d > 0) {
                    /* src is a set of columns */
                    std::vector<Cell *> cVect =
                        ((Column *)inputBits[y][x])->GetCells();
                    projections.insert(
                        projections.end(),
                        cVect.begin(),
                        cVect.end()
                    );
                } else {
                    projections.push_back(inputBits[y][x]);
                }
                //printf("\t\tchecking input projections...\n");
                for (unsigned k=0; k<projections.size() && !foundSampleSize; k++) {
                    /*
                     * The cla wants to form new synapses with cells that would
                     * have been able to predict this cell's activity.
                     */
                    bool activeflag =
                        motorSrc ? projections[k]->IsActive() :
                                   projections[k]->WasActive();
                    if (projections[k]->WasLearning() && activeflag) {
                        //printf("\t\t\t(col %d,%d, c %d)\n", x, y, k);
                        Synapse *newSyn = new Synapse(
                            projections[k], x, y, motorSrc
                        );
                        //printf("\t\tnew synapse 0x%08x\n", newSyn);
                        seg->NewSynapse(newSyn);
                        //printf("\t\tsynapse added to segment\n");
                        // check if we have found the subset sample size.
                        if ((++synsFound) >= subsampleSz)
                            foundSampleSize = true;
                    }
                }
            }
        }
    }

    //printf("\t\tformed %d synapses.\n", synsFound);
    return true;
}

void Cell::RemoveSegment(int segidx)
{
    DistalDendriteSegments.erase(DistalDendriteSegments.begin()+segidx);
}

// return the segment with the most active synapses in the current timestep.
// the "current" timestep is determined by the context of the calling function.
// returns NULL if there were no segments on this cell yet.
DendriteSegment* Cell::GetMostActiveSegment()
{
    int mostSynapseCount = -1;
    DendriteSegment *mostActiveSeg = NULL;
    // if no segments yet exist, then return NULL.
    int numSegs = this->GetNumSegments();
    for (int i=0; i<numSegs; i++) {
        int synCount = DistalDendriteSegments[i]->GetNumIsActiveSynapses();
        if (synCount > mostSynapseCount) {
            mostSynapseCount = synCount;
            mostActiveSeg = DistalDendriteSegments[i];
        }
    }
    return mostActiveSeg;
}

/*
 * Return the segment with the highest number of synapses that are near or
 * above the activation threshold, or NULL if none exist.
 */
DendriteSegment* Cell::GetBestMatchingSegment(
    int *bestSegIdx,
    HtmSublayer *sublayer)
{
    int synCount = 0, mostSynCount = -1;
    DendriteSegment *bestSeg = NULL;

    /*
     * To be considered as the best match, a segment must have a minimum number
     * of active, sensory synapses that are at least nearly connected. Motor
     * synapses may also contribute to segment activation.
     */
    int lastActiveColumns = sublayer->LastActiveColumns();
    int minThreshold;
    SensoryRegion *mp = sublayer->GetLower()->GetMotorPattern();

    if (parentColumn->IsSensorimotorColumn() && mp) {
        minThreshold = 
            (lastActiveColumns + mp->GetNumActiveInputs()) *
            DendriteSegment::GetSubsamplePercent();
    } else {
        minThreshold = lastActiveColumns *
            DendriteSegment::GetSubsamplePercent();
    }

    std::vector<DendriteSegment *>::iterator beg = DistalDendriteSegments.begin();
    std::vector<DendriteSegment *>::iterator end = DistalDendriteSegments.end();
    int x=0;
    for (std::vector<DendriteSegment *>::iterator i = beg; i != end; i++, x++) {
        synCount = (*i)->GetNumIsNearActiveSynapses();
        printf("\t\t\tsegment has %d near active, %d sensory syns\n",
            synCount, (*i)->GetNumSensorySyns());
        printf("\t\t\tthreshold %d\n", minThreshold);
        if (synCount >= minThreshold)
            if (synCount > mostSynCount) {
                mostSynCount = synCount;
                bestSeg = *i;
                *bestSegIdx = x;
            }
    }

//    printf("segment %d\n", x);
    return bestSeg;
}

std::vector<DendriteSegment *> Cell::GetSegments()
{
    return DistalDendriteSegments;
}

unsigned int Cell::GetNumSegments()
{
    return DistalDendriteSegments.size();
}

