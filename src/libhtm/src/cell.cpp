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
    memset(&learning[0], false, sizeof(bool)*2);
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
 *    motor patterns (er, or allocentric signal?).
 */
DistalDendrite* Cell::NewSegment(HtmSublayer *sublayer, bool FirstPattern)
{
    if (FirstPattern && !sublayer->IsSensorimotor()) {
        printf("\t\t[cell] first patt & not smi, so not adding "
               "segment.\n");
        return NULL;
    }

    DistalDendrite *newSeg = new DistalDendrite();
    bool synsFound = false;

    if (sublayer->IsSensorimotor()) {
        GenericSublayer *inputSignals = sublayer->GetLower();
        GenericSublayer *mp = inputSignals->GetMotorPattern();
        GenericSublayer *lp = inputSignals->GetLocationPattern();
        //printf("Loc patt %08x\n", lp);
        //sublayer->GetHtmPtr()->PrintPattern(mp);
        if (mp) {
            //printf("\t"
            //    "Adding synapses to distal dendrite "
            //    "segment in SMI layer.\n"
            //);
            synsFound = AddSynapsesFromSublayer(
                sublayer, lp, LOCATION_DISTAL, newSeg
            );
            //synsFound = AddSynapsesFromSublayer(
                //sublayer, mp, MOTOR_DISTAL, newSeg
            //);
            /*printf(
                "\t" \
                "Adding synapses to distal dendrite from previous "
                "sensory pattern."
            "\n");*/
        } else {
            printf("\tNo motor pattern found.\n");
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
    DistalDendrite *seg)
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
     * for connections to motor input, the dendrite
     * segment subsamples the number of active input bits in the
     * input pattern.
     *
     * location pattern inputs follow the same rule as motor.
     */
    int subsampleSz =
        (inType == LATERAL_DISTAL ?
            thisSublayer->LastActiveColumns() :
            src->GetNumActiveInputs()) *
        DendriteSegment::GetSubsamplePercent();
    if (inType == SENSORY_DISTAL)
        subsampleSz *= 0.50;

    // when adding synapses from the location signal,
    // disregard relative position of this output cell
    // in relation to input pattern.
    if (inType == LOCATION_DISTAL) {
        double sqrtw = sqrt(w);
        for (int i=0; i<h&&!foundSampleSize; i++) {
            for (int j=0; j<w&&!foundSampleSize; j++) {
                if (inputBits[i][j]->IsActive()) {
                    Synapse *newSyn = new Synapse(
                        inputBits[i][j], j%(int)sqrtw, j/(int)sqrtw, inType
                    );
                    //printf("\t\tnew synapse 0x%08x\n", newSyn);
                    seg->NewSynapse(newSyn);
                    if ((++synsFound) >= subsampleSz)
                        foundSampleSize = true;
                }
           }
        }
    } else {
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
                            inType == LATERAL_DISTAL ?
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
    }

    //printf("\t\tformed %d synapses.\n", synsFound);
    return true;
}

void Cell::RefreshDendrites(GenericSublayer *NewPattern)
{
    for (unsigned int s=0; s<DistalDendriteSegments.size(); s++) {
        //printf("refreshing dendrite %u\n", s);
        DistalDendriteSegments[s]->RefreshSynapses(NewPattern);
    }
}

void Cell::RemoveSegment(int segidx)
{
    DistalDendriteSegments.erase(DistalDendriteSegments.begin()+segidx);
}

/*
 * return the segment with the most active synapses in the current timestep.
 * returns NULL if there were no segments on this cell yet.
 */
DistalDendrite* Cell::GetMostActiveSegment()
{
    int mostSynapseCount = -1;
    DistalDendrite *mostActiveSeg = NULL;
    // if no segments yet exist, then return NULL.
    unsigned int numSegs = GetNumSegments();
    //printf("\t\tcell has %d segments\n", numSegs);
    for (unsigned int i=0; i<numSegs; i++) {
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
DistalDendrite* Cell::GetBestMatchingSegment(
    int *bestSegIdx,
    HtmSublayer *sublayer)
{
    int synCount = 0, mostSynCount = -1;
    DistalDendrite *bestSeg = NULL;

    /*
     * To be considered as the best match, a segment must have a minimum number
     * of active, sensory synapses that are at least nearly connected. Motor
     * synapses may also contribute to segment activation.
     */
    int lastActiveColumns = sublayer->LastActiveColumns();
    int minThreshold;
    SensoryRegion *mp = sublayer->GetLower()->GetMotorPattern();

    if (sublayer->IsSensorimotor() && mp) {
        minThreshold = 
            (lastActiveColumns + mp->GetNumActiveInputs()) *
            DendriteSegment::GetSubsamplePercent();
    } else {
        minThreshold = lastActiveColumns *
            DendriteSegment::GetSubsamplePercent();
    }

    std::vector<DistalDendrite *>::iterator beg = DistalDendriteSegments.begin();
    std::vector<DistalDendrite *>::iterator end = DistalDendriteSegments.end();
    int x=0;
    for (std::vector<DistalDendrite *>::iterator i = beg; i != end; i++, x++) {
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

