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
#include "synapse.h"

Cell::Cell(Column *col, unsigned int idx)
{
    parentColumn = col;
    colIdx = idx;
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
 * 1. For L2/3 sublayers, distal segments only synapse with
 *    lateral cells within the same layer (although, in real
 *    brains, axons from L2/3 of the contralateral cortical
 *    hemisphere are also used).
 * 2. For L4 sublayers, distal dendrite segments synapse with
 *    two sources of input: feed-forward sensory patterns, and
 *    feed-forward motor patterns.
 */
DendriteSegment* Cell::NewSegment(HtmSublayer *sublayer, bool FirstPattern)
{
    DendriteSegment *newSeg = new DendriteSegment(sublayer->IsSensorimotor());
    bool synsFound = false;

    /*
     * The first pattern will never be within a sensorimotor
     * context.
     */
    if (FirstPattern) {
        newSeg->SetNoTemporalContext();
        DistalDendriteSegments.push_back(newSeg);
        return NULL;
    }

    if (sublayer->IsSensorimotor()) {
        //printf("Sublayer is sensorimotor.\n");
        GenericSublayer *sp = sublayer->GetPreviousInputPattern();
        GenericSublayer *mp = sp->GetMotorPattern();
        //sublayer->GetHtmPtr()->PrintPattern(mp);
        if (mp) {
            /*printf("\t"
                "Adding synapses to distal dendrite from previous "
                "motor pattern."
            "\n");*/
            synsFound = AddSynapsesFromSublayer(
                sublayer, mp, MOTOR_DISTAL, newSeg
            );
            /*printf(
                "\t" \
                "Adding synapses to distal dendrite from previous "
                "sensory pattern."
            "\n");*/
            synsFound += AddSynapsesFromSublayer(
                sublayer, sublayer, SENSORY_DISTAL, newSeg
            );
        } else {
            printf("\tNo motor pattern found.\n");
            newSeg->SetNoTemporalContext();
            DistalDendriteSegments.push_back(newSeg);
            return NULL;
        }
    } else {
        //printf("\tSublayer is high-order.\n");
        /*printf(
            "\tAdding synapses on distal dendrite to lateral cells.\n"
        );*/
        synsFound = AddSynapsesFromSublayer(
            sublayer, sublayer, LATERAL_DISTAL, newSeg
        );
    }

    /*
     * if zero synapses were formed, then there was no previous activity
     * in the sequence so skip adding a new segment.
     */
    if (synsFound) {
        //printf("\t\tfound %d cells\n", synsFound);
        DistalDendriteSegments.push_back(newSeg);
    } else {
        //printf("\t\tDid not find any previous activity.\n");
        delete newSeg;
        newSeg = NULL;
    }

    return newSeg;
}

bool Cell::AddSynapsesFromSublayer(
    HtmSublayer *thisSublayer,
    GenericSublayer *src,
    input_t inType,
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

    /*
     * subsampling on distal dendrite segments in the temporal
     * memory depends on the source of input bits.
     *
     * for lateral, intra-layer connections, it subsamples the
     * number of active columns in the previous timestep.
     *
     * for connections to sensory or motor input, the dendrite
     * segment subsamples the number of active input bits in the
     * input pattern.
     */
    int subsampleSz =
        (inType!=MOTOR_DISTAL ?
            thisSublayer->LastActiveColumns() :
            src->GetNumActiveInputs()) *
        DendriteSegment::GetSubsamplePercent();

    std::vector<GenericInput *> projections;

    auto il = { h-1-celly, celly, w-1-cellx, cellx };

    /*printf("\t\tsubsampleSz = %d * %f = %d\n",
            inType!=MOTOR_DISTAL ?
                thisSublayer->LastActiveColumns() :
                src->GetNumActiveInputs(),
            DendriteSegment::GetSubsamplePercent(), subsampleSz
    );*/
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
                 * Synapses from HtmSublayer to SensoryRegion connect to
                 * inputBits[y][x].
                 *
                 * Synapses from HtmSublayer to HtmSublayer connect to the
                 * learning cell in inputBits[y][x].
                 */
                projections.clear();
                if (inType == LATERAL_DISTAL) {
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
                //printf("\t\tchecking %u input projections...\n", projections.size());
                for (unsigned k=0; k<projections.size() && !foundSampleSize; k++) {
                    /*
                     * The cla wants to form new synapses with cells that would
                     * have been able to predict this cell's activity.
                     */
                    bool activeflag =
                        inType != MOTOR_DISTAL ?
                            projections[k]->WasActive() :
                            projections[k]->IsActive();
                    if (activeflag) {
                        if (inType == LATERAL_DISTAL)
                            if (!projections[k]->WasLearning())
                                continue;
                        //printf("\t\t\t(col %d,%d, c %d)\n", x, y, k);
                        Synapse *newSyn = new Synapse(
                            projections[k], x, y, inType
                        );
                        //printf("\t\tnew synapse 0x%08x\n", newSyn);
                        seg->NewSynapse(newSyn);
                        //printf("\t\tsynapse added to segment\n");
                        /*
                         * check if we have found the subset
                         * sample size.
                         */
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
        //printf("\t\t\tsegment has %d near active, %d sensory syns\n",
            //synCount, (*i)->GetNumSensorySyns());
        //printf("\t\t\tthreshold %d\n", minThreshold);
        if (synCount >= minThreshold)
            if (synCount > mostSynCount) {
                mostSynCount = synCount;
                bestSeg = *i;
                *bestSegIdx = x;
            }
    }

    //printf("segment %d\n", x);
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

