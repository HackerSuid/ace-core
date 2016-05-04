#include <cstddef>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include "htmsublayer.h"
#include "column.h"
#include "cell.h"
#include "dendritesegment.h"
#include "synapse.h"

Cell::Cell(Column *col)
{
    parentColumn = col;
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

bool Cell::IsLearning()
{
    return learning[0];
}

bool Cell::WasLearning()
{
    return learning[1];
}

void Cell::SetLearning(bool flag)
{
    learning[1] = learning[0];
    learning[0] = flag;
}

// returns the segment that got added to the cell so it can be queued
// up for reinforcement or discarded.
DendriteSegment* Cell::NewSegment(HtmSublayer *sublayer, bool FirstPattern)
{
    DendriteSegment *newSeg = new DendriteSegment;

    if (FirstPattern) {
        /*
         * First pattern of sequence just creates an empty segment to reserve
         * a cell to represent it.
         */
        //newSeg = NULL;
//        printf("\t\tfirst pattern, no populated segment\n");
        newSeg->SetNoTemporalContext();
        DistalDendriteSegments.push_back(newSeg);
        return NULL;
    }

    Column ***columns = (Column ***)sublayer->GetInput();
    int w = sublayer->GetWidth();
    int h = sublayer->GetHeight();
    int d = sublayer->GetDepth();

    int cellx = parentColumn->GetX();
    int celly = parentColumn->GetY();
    // search through all the cells within the sublayer for cells that were
    // previously active and learning a pattern in context, and create a new
    // segment with connections to a subset of these cells.
    //
    // biologically, each cell only receives axons from other cells nearby,
    // not globally.
    bool foundSampleSize = false;
    int synsFound = 0;
    int subsampleSz = sublayer->LastActiveColumns() *
                      DendriteSegment::GetSubsamplePercent();
//    printf("\t\tcreating %d synapses for cell in column (%d,%d)\n",
//        subsampleSz, cellx, celly);
    //printf("subsampleSz = %d * %f = %d\n",
        //sublayer->LastActiveColumns(),
        //DendriteSegment::GetSubsamplePercent(), subsampleSz
    //);
    auto il = { h-1-celly, celly-0, w-1-cellx, cellx-0 };
    for (int distance=1; distance<std::max(il); distance++) {
        for (int yd=-distance; yd<=distance&&!foundSampleSize; yd++) {
            for (int xd=-distance; xd<=distance&&!foundSampleSize; xd++) {
                if (abs(xd) != distance && abs(yd) != distance)
                    continue;
//                printf("\ttrying (%d, %d)...\n", cellx+xd, celly+yd);

                int x = cellx+xd;
                int y = celly+yd;
                // minimum limitations
                if ((x<0 || y<0) || (x==cellx && y==celly))
                    continue;
                /*if (x < 0) {
                    // avoid repeating positions when (x, y) isn't
                    // going to change.
                    if (yd > -d)
                        continue;
                    x = 0;
                }
                if (y < 0)
                    y = 0;*/
                // maximum limitations
                if ((x>=w) || (y>=h))
                    continue;
                //if (x >= w)
                //    x = w-1;
                //if (y >= h)
                //    y = h-1;
        //for (int i=0; i<w && !foundSampleSize; i++) {
            //for (int j=0; j<h && !foundSampleSize; j++) {
                std::vector<Cell *> cells =
                    columns[y][x]->GetCells();
//                printf("\t\tchecking cells...\n");
                for (int k=0; k<d && !foundSampleSize; k++) {
                    /*
                     * The cla wants to form new synapses with cells that would
                     * have been able to predict this cell's activity.
                     */
                    if (cells[k]->WasLearning() && cells[k]->WasActive()) {
//                        printf("\t\t\t(col %d,%d, c %d)\n", x, y, k);
                        Synapse *newSyn = new Synapse(
                            cells[k], x, y
                        );
    //                    printf("\t\tnew synapse 0x%08x\n", newSyn);
                        newSeg->NewSynapse(newSyn);
                        //printf("\t\tsynapse added to segment\n");
                        // check if we have found the subset sample size.
                        if ((++synsFound) >= subsampleSz)
                            foundSampleSize = true;
                    }
                }
            }
        }
    }

    // if zero synapses were formed, then there was no previous activity
    // in the sequence so skip adding a new segment.
    if (synsFound > 0) {
//        printf("\t\tfound %d cells\n", synsFound);
        DistalDendriteSegments.push_back(newSeg);
    } else {
//        printf("\t\tDid not find any previous activity.\n");
        delete newSeg;
        newSeg = NULL;
    }

    return newSeg;
}

// return the segment with the most active synapses in the current timestep.
// the "current" timestep is determined by the context of the calling function.
// returns NULL if there were no segments on this cell yet.
DendriteSegment* Cell::GetMostActiveSegment()
{
    int mostSynapseCount = -1;
    DendriteSegment *mostActiveSeg = NULL;
    // if no segments yet exist, we will return NULL.
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
DendriteSegment* Cell::GetBestMatchingSegment(int lastActiveColumns)
{
    int synCount = 0, mostSynCount = -1;
    DendriteSegment *bestSeg = NULL;

    /*
     * To be considered as the best match, a segment must have a minimum number
     * of active synapses that are at least nearly connected.
     */
    int minThreshold = lastActiveColumns * DendriteSegment::GetSubsamplePercent();

    std::vector<DendriteSegment *>::iterator beg = DistalDendriteSegments.begin();
    std::vector<DendriteSegment *>::iterator end = DistalDendriteSegments.end();
    int x=0;
    for (std::vector<DendriteSegment *>::iterator i = beg; i != end; i++, x++) {
        synCount = (*i)->GetNumIsNearActiveSynapses();
        if (synCount >= minThreshold)
            if (synCount > mostSynCount) {
                mostSynCount = synCount;
                bestSeg = *i;
            }
    }

//    printf("segment %d\n", x);
    return bestSeg;
}

std::vector<DendriteSegment *> Cell::GetSegments()
{
    return DistalDendriteSegments;
}

int Cell::GetNumSegments()
{
    return DistalDendriteSegments.size();
}

