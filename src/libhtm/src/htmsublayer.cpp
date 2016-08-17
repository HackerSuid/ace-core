#include <stdlib.h>
#include <cstdio>
#include <algorithm>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "htm.h"
#include "htmsublayer.h"
#include "sensoryinput.h"
#include "sensoryregion.h"
#include "column.h"
#include "cell.h"
#include "dendritesegment.h"
#include "synapse.h"
#include "util.h"

HtmSublayer::HtmSublayer(
    unsigned int h,
    unsigned int w,
    unsigned int cpc,
    Htm *htmPtr,
    bool sensorimotorFlag
) {
    height = h;
    width = w;
    depth = cpc;
    this->htmPtr = htmPtr;
    sensorimotorLayer = sensorimotorFlag;
    memset(&numActiveColumns[0], 0, 2);
}

HtmSublayer::~HtmSublayer()
{
}

void HtmSublayer::AllocateColumns(
    float rfsz,
    float localActivity,
    float columnComplexity,
    bool highTier,
    int activityCycleWindow)
{
    rec_field_sz = height*width*rfsz;
    this->localActivity = localActivity;
    input = (GenericInput ***)malloc(sizeof(GenericInput **) * height);
    for (unsigned int i=0; i<height; i++) {
        input[i] = (GenericInput **)malloc(sizeof(GenericInput *) * width);
        for (unsigned int j=0; j<width; j++)
            input[i][j] = new Column(
                j, i,
                this->depth,
                rec_field_sz,
                localActivity,
                columnComplexity,
                highTier,
                activityCycleWindow
            );
    }
}

// the columns have natural centers over the input. this means their receptive
// fields will favor connecting to a topographically similar area within the
// afferent region. synapses are formed randomly within the receptive field
// which will result in the columns having the attribute of partially shifted
// overlap.
void HtmSublayer::InitializeProximalDendrites()
{
    Column ***columns = (Column ***)input;

    for (unsigned int i=0, n=0; i<height; i++)
        for (unsigned int j=0; j<width; j++, n++)
            columns[i][j]->InitializeProximalDendrite(
                lower,
                lower->GetWidth()/width,
                lower->GetHeight()/height
            );
}

void HtmSublayer::RefreshLowerSynapses()
{
    Column ***columns = (Column ***)input;

    for (unsigned int i=0, n=0; i<height; i++)
        for (unsigned int j=0; j<width; j++, n++)
            columns[i][j]->RefreshNewPattern(lower);
}

// Currently this algorithm represents L2/3 in cortex.
void HtmSublayer::CLA(bool Learning, bool allowBoosting)
{
    numActiveColumns[0] = 0;
    /*
     * Spatial Pooler
     *
     * Compute the prediction-assisted, inference of feedforward input (overlap
     * score)  for each column. This is a linear summation of active,
     * feedforward input bits plus a. The columns found to have the highest overlap with
     * the input pattern perform an inhibition function to prevent neighboring
     * columns within a particular radius from becoming active. The columns
     * learn to map spatially similar input patterns to the same or a similar
     * set of active columns. This step results in a certain level of spatial
     * feature generalization, depending on the topography of the input.
     */
    SpatialPooler(Learning, allowBoosting);
    /*
     * Sequence (Temporal) Memory
     * 
     * 1. Within columns with active proximal dendrite segments, activate cells
     *    that are already partially depolarized, or all of them if none are
     *    depolarized in order to contextualize the sdr given previous activity.
     *    Create new dendrite segments on cells chosen to learn to predict
     *    the current feedforward input pattern.
     * 2. Form a prediction given the lateral, intrinsic connections of the
     *    region by depolarzing cells with active distal dendrite segments.
     *
     * Note: if this is the first input pattern of the sequence, then temporal
     * learning, inference, and prediction is skipped. The first pattern in a
     * sequence physically has no context.
     */
    SequenceMemory(Learning, htmPtr->FirstPattern());

    // Update the internal timestep counter for every column.
    NewTimestep();
}

void HtmSublayer::SpatialPooler(bool Learning, bool allowBoosting)
{
    Column ***columns = (Column ***)input;

    /*
     * Compute the average connected receptive field radius. This value is
     * used in determining the lateral neighborhood of each column.
     */
    _ComputeInhibitionRadius(columns);

    /*
     * Compute the overlap score of each column, which may be competitively
     * boosted. Column activations are boosted when a column does not become
     * active often enough and falls below the minimum threshold. This will
     * happen if the overlap exceeds the minimum to fire but isn't strong
     * enough to ever avoid being inhibited, or its synapses never become
     * connected even though there may be enough activity within its receptive
     * field. The purpose of boosting is to cause all the columns to compete
     * in representing a pattern, which in turn creates redundancy in the event
     * that columns need to adjust their receptive fields. More importantly,
     * it guarantees that "poor, starved" columns will get to represent at
     * least some patterns so that "greedy" columns can't try to represent too
     * many.
     */
    for (unsigned int i=0; i<height; i++) {
        for (unsigned int j=0; j<width; j++) {
            columns[i][j]->ComputeOverlap();
            int minOverlap = columns[i][j]->GetMinOverlap();
            /*
             * High Tiering comes in because of this. The boost may cause
             * "starved" columns to displace columns that better represent
             * the input patterns. Boosting is meant just for helping keep
             * "hungry" columns satisfied and representing at least some
             * patterns.
             */
            if (columns[i][j]->GetOverlap() < minOverlap)
                columns[i][j]->SetOverlap(0, Learning);
            else
                columns[i][j]->SetOverlap(
                    columns[i][j]->GetOverlap()*columns[i][j]->GetBoost(),
                    Learning
                );
        }
    }
    /*
     * Inhibit the neighbors of the columns which received the highest level of
     * feedforward activation.
     */
    for (unsigned int i=0; i<height; i++) {
        for (unsigned int j=0; j<width; j++) {
            if (columns[i][j]->GetOverlap()) {
                if (_EligibleToFire(columns[i][j])) {
                    columns[i][j]->SetActive(true, Learning);
                    numActiveColumns[0]++;
                    // modify synapses of active columns to reinforce learning the patterns.
                    if (Learning)
                        columns[i][j]->ModifySynapses();
                } else
                    columns[i][j]->SetActive(false, Learning);
            } else
                columns[i][j]->SetActive(false, Learning);
        }
    }
    /*
     * Update boosting parameters if htm is learning.
     */
    if (Learning && allowBoosting) {
        for (unsigned int i=0; i<height; i++) {
            for (unsigned int j=0; j<width; j++) {
                /*columns[i][j]->GetNeighbors(
                    (Column ***)input,
                    inhibitionRadius,
                    (int)width, (int)height
                );*/
                double minActivityLevel = columns[i][j]->GetMinActivityLevel();
                columns[i][j]->UpdateActivationBoost(minActivityLevel);
                columns[i][j]->UpdateOverlapBoost(minActivityLevel);
            }
        }
    }
}

void HtmSublayer::SequenceMemory(bool Learning, bool firstPattern)
{
    Column ***columns = (Column ***)input;
    /*
     * These vectors are used as temporary storage of the column and cell
     * states for the current timestep, and they are used as references to 
     * modify the corresponding state machines after the current timestep has
     * been processed by all cells.
     */
    std::vector<Column *> inactiveColumns;
    std::vector<Cell *> predictedCells, nonPredictedCells;
    std::vector<Cell *> learningCells, nonLearningCells;

    unsigned long numActiveColsPredicted=0, numInactiveColsPredicted=0;
    /*
     * For each active column:
     * 1. Compute the active and learning state of each cell in the column for
     *    the current timestep. The active and learning states are computed using
     *    the temporary vectors but the corresponding state machines are not
     *    updated until each cell has had a chance to analyze the current state
     *    of all the cells. The active state represents whether or not the cells
     *    are firing an action potential, and the learning state indicates that
     *    the cell is chosen to represent the feed-forward input in a unique
     *    context. The algorithm is working on the set of active columns after
     *    the spatial pooling phase.
     */
    for (unsigned int i=0; i<height; i++) {
        for (unsigned int j=0; j<width; j++) {
            int nc = columns[i][j]->GetNumCells();
            std::vector<Cell *> cells = columns[i][j]->GetCells();
            if (columns[i][j]->IsActive()) {
//                printf("Col [%d,%d] active\n", j, i);
                /*
                 * Search for cells that were predicted and
                 * activate them; they fire and inhibit ipsicolumnar cells. If
                 * cells that caused this cell to be predicted were learn
                 * cells (as opposed to bursting), then they were part of a
                 * correct sequence prediction, so choose this cell as the
                 * learning cell for this column. If no cells were predicting
                 * the current feedforward input seen by this column, activate
                 * all the cells as if any temporal context is valid. Update
                 * the learn state vector for each cell along the way.
                 */
                bool buPredicted = false;
                bool learnCellChosen = false;
                for (int k=0; k<nc; k++) {
                    /*
                     * If a cell is predicting the activity of this column for
                     * the current timestep, activate it; the cell made a
                     * correct prediction. Otherwise, the predicting cell will
                     * be negatively reinforced later.
                     */ 
                    if (cells[k]->IsPredicted()) {
//                        printf("\tpredicted by cell %d\n", k);
                        buPredicted = true;
                        numActiveColsPredicted++;
                        predictedCells.push_back(cells[k]);
                        /*
                         * Get the segment that caused the cell to become
                         * predicted. It will be the one that had the most
                         * active synapses in the previous timestep. There may
                         * be more than one, so just get the best.
                         *
                         * Note: at this point in the algorithm, the active cells
                         * that caused this prediction are still "active" during
                         * this timestep from the point of view of the state
                         * machines. So the code will call IsFiring() to get
                         * the cells that were active during the previous
                         * timestep.
                         */
                        DendriteSegment *ActiveSegment = cells[k]->GetMostActiveSegment();
                        std::vector<Synapse *> predSyns = ActiveSegment->GetSynapses();
//                        printf("\t\tpredicted by synapses:\n");
//                        for (unsigned int s=0; s<predSyns.size(); s++)
//                            printf("\t\t(%d,%d)\n", predSyns[s]->GetX(), predSyns[s]->GetY());
                        /*
                         * Note: as with computing the active state, it appears
                         * the current timestep is used to check the learning
                         * state of cells connected to the segment, but since
                         * the state machine has not been updated at this point,
                         * the algorithm ultimately calls IsLearning() to get
                         * the learn state for the cells in the previous timestep.
                         */
                        if (ActiveSegment->IsActiveFromLearning()) {
//                            printf("\tcell %d predicted and chosen for learning\n", k);
                            learningCells.push_back(cells[k]);
                            learnCellChosen = true;
                        } else {
//                            printf("cell %d predicted BUT NOT chosen for learning\n", k);
                        }
                    } else
                        nonPredictedCells.push_back(cells[k]);
                }
                /*
                 * If no cell was predicted in this column, then all cells
                 * represent valid predictions, so "burst" the column.
                 * Otherwise deactivate the non-predicted cells.
                 */
                if (!buPredicted) {
//                    printf("\tno cells predicted; activating all cells\n");
                    for (int k=0; k<nc; k++) {
                        /*
                         * It was just added to the non-prediction list, which
                         * will not be activated, so move it.
                         */
                        nonPredictedCells.pop_back();
                        predictedCells.push_back(cells[k]);
                    }
                }
                /*
                 * If no cells were predicted due to learning cells, then choose
                 * the best matching cell as the new learning cell. This cell
                 * will represent the feed forward input within the current
                 * temporal context. The other cells get learn state set to
                 * false for this timestep.
                 *
                 */
                if (!learnCellChosen) {
                    //printf("\tchoosing learning cell\n");
                    DendriteSegment *segment = NULL;
                    int segidx;
                    Cell *BestCell = columns[i][j]->GetBestMatchingCell(
                        &segment,
                        &segidx,
                        this->LastActiveColumns(),
                        firstPattern
                    );
                    /* BestCell will be the only cell learning this transition. */
                    learningCells.push_back(BestCell);
                    /*
                     * Queue up the segment of BestCell to be reinforced.
                     */
                    //printf("\tenqueuing best cell segment 0x%08x\n", segment);
                    _EnqueueSegmentUpdate(BestCell, segidx, segment, false);
                }
                /*
                 * After the learning cell is chosen, all other cells in the
                 * columns are set as non-learning. The last
                 * element of learningCells will be the learning
                 * cell for this column.
                 */
                for (int x=0; x<nc; x++)
                    if (cells[x] != learningCells.back())
                        nonLearningCells.push_back(cells[x]);
            } else {
                inactiveColumns.push_back(columns[i][j]);
                // Add new data point for prediction specificity.
                for (int c=0; c<nc; c++) {
                    if (cells[c]->IsPredicted()) {
                        numInactiveColsPredicted++;
                        break;
                    }
                }
            }
        }
    }
    // Update running average of prediction stability. Ignore the first
    // pattern statistics.
    if (!firstPattern) {
        // comp and spec windows are the same sizr.
        if (predCompWindow.size() == PRED_COMP_WINDOW_SZ) {
            predCompWindow.pop_front();
            predSpecWindow.pop_front();
        }
        predCompWindow.push_back(
            ((float)numActiveColsPredicted) /
            ((float)(CurrentActiveColumns()>0 ? CurrentActiveColumns() : 1.0))
        );
        unsigned long totalColsPredicted =
            numActiveColsPredicted + numInactiveColsPredicted;
        /*printf("numActiveColsPredicted=%u + "
               "numInactiveColsPredicted=%u = "
               "totalColsPredicted=%u \n"
               "Pushing %u / %u = %f\n",
            numActiveColsPredicted,
            numInactiveColsPredicted,
            totalColsPredicted>0 ? totalColsPredicted : 1,
            numActiveColsPredicted,
            totalColsPredicted>0 ? totalColsPredicted : 1,
            ((float)numActiveColsPredicted) /
            ((float)(totalColsPredicted>0 ? totalColsPredicted : 1.0)));*/
        predSpecWindow.push_back(
            ((float)numActiveColsPredicted) /
            ((float)(totalColsPredicted>0 ? totalColsPredicted : 1.0))
        );
    }
    /*
     * Update active, predictive and learning state machine for each cell in
     * both active and inactive columns.
     */
    while (predictedCells.size()) {
        predictedCells.back()->SetActive(true);
        predictedCells.pop_back();
    }
    while (nonPredictedCells.size()) {
        nonPredictedCells.back()->SetActive(false);
        nonPredictedCells.pop_back();
    }
    while (learningCells.size()) {
        learningCells.back()->SetLearning(true);
        learningCells.pop_back();
    }
    while (nonLearningCells.size()) {
        nonLearningCells.back()->SetLearning(false);
        nonLearningCells.pop_back();
    }
    for (int i=0; i<(int)inactiveColumns.size(); i++) {
        std::vector<Cell *> cells = inactiveColumns[i]->GetCells();
        int nc = inactiveColumns[i]->GetNumCells();
        for (int k=0; k<nc; k++) {
            cells[k]->SetActive(false);
            cells[k]->SetLearning(false);
        }
    }
    /*
     * 2. Original implementation:
     *
     *    Compute the predictive state of each cell for the current timestep.
     *    Learned predictions are reinforced here. The cells' learn states are
     *    disregarded when just generating a prediction because a bursting
     *    column should generate multiple predictions, which always happens.
     *
     *    Modified implementation:
     *
     *    If the HTM is learning sequences, predictions are only generated
     *    for cells active from lateral, learning cells. So bursting does
     *    not generate extra predictions from non-learning active cells.
     */
//    printf("Computing predictions\n");
    for (unsigned int i=0; i<height; i++) {
        for (unsigned int j=0; j<width; j++) {
            std::vector<Cell *> cells = columns[i][j]->GetCells();
            int nc = columns[i][j]->GetNumCells();
            for (int k=0; k<nc; k++) {
                /*
                 * If a cell is being predicted for the next timestep, set it
                 * into the predictive state (depolarize it). Otherwise leave
                 * it at its resting potential.
                 */
                std::vector<DendriteSegment *> segments = cells[k]->GetSegments();
                int numSegs = cells[k]->GetNumSegments();
                bool predflag = false;
                for (int s=0; s<numSegs; s++) {
                    /*
                     * Only SetPredicted() once if there are multiple active
                     * segments to avoid corrupting the state machine.
                     */
                    bool segactive =
                        Learning ? segments[s]->IsActiveFromLearning() :
                                   segments[s]->IsActive();
                    if (segactive and !predflag) {
//                        printf("\tcol (%d, %d) cell %d [0x%08x] is predicting\n", j, i, k, cells[k]);
                        //std::vector<Synapse*> syns = segments[s]->GetIsActiveSynapses();
                        //int n = segments[s]->GetNumIsActiveSynapses();
//                        printf("\t\t");
//                        for (int f=0; f<n; f++) {
//                            printf("[%d,%d] ", syns[f]->GetX(), syns[f]->GetY());
//                        printf("\n");
                        cells[k]->SetPredicted(true);
                        predflag = true;
                        _EnqueueSegmentUpdate(cells[k], s, segments[s], true);
                    }
                }
                if (!predflag)
                    cells[k]->SetPredicted(false);
            }
        }
    }
    
    /*
     * 3. Modify (positively or negatively) distal dendrite synapses based on
     *    prediction accuracy. Segment updates only get enqueued if a cell is
     *    chosen for learning that had a new segment created or a prediction
     *    is generated from existing segments. Only synapses connected between
     *    learning cells are modified to avoid a bursting column from causing
     *    baseless predictions.
     */
//    printf("Modifying synapses...\n");
    int segmentUpdateListSize = segmentUpdateList.size();
    int skipSegments = 0;
//    printf("\t%d segment updates...\n", segmentUpdateListSize);
    for (int i=0; i<segmentUpdateListSize; i++) {
        /*
         * Start at the front and skip the ones that are staying in the queue.
         */
        SegmentUpdate *nextUpdate = segmentUpdateList.at(skipSegments);
        Cell *cellUpdate = nextUpdate->GetCell();
        if (cellUpdate->IsLearning()) {
            /*
             * The cell was either successfully predicted to be active by other
             * learning cells, or it was chosen as the best learning cell to
             * predict the current input pattern.
             */
//            printf("Dequeing 0x%08x: Learning cell\n", nextUpdate);
            _DequeueSegmentUpdate(nextUpdate, true);
        } else if (cellUpdate->WasPredicted() and
            !cellUpdate->IsActive()) {
            /*
             * The cell was predicted for the current timestep but the column
             * did not become active, meaning an incorrect prediction of the
             * feed-forward input.
             */
//            printf("Dequeing 0x%08x: Bad prediction\n", nextUpdate);
            _DequeueSegmentUpdate(nextUpdate, false);
        } else {
            /*
             * A segment update will be skipped for cells that became predicted
             * this timestep, so they aren't updated until next timestep when
             * it is known whether the prediction was correct or not.
             */
            skipSegments++;
        }
    }
//    printf("\n\n");
}

/*
 * A queue of SegmentUpdate structures is used to track segments that need to
 * have their synapses modified. Accurately predicted cells will have their
 * synapses strengthened, and cells that did not become active after being
 * predicted be weakened.
 */
void HtmSublayer::_EnqueueSegmentUpdate(
    Cell *cell,
    int segidx,
    DendriteSegment *segment,
    bool connected)
{
    SegmentUpdate *newSegment = new SegmentUpdate(cell, segidx, segment, connected);

    segmentUpdateList.push_back(newSegment);
}

void HtmSublayer::_DequeueSegmentUpdate(
    SegmentUpdate *segUpdate,
    bool reinforce)
{
    DendriteSegment *segment = segUpdate->GetSegment();
    int segidx = segUpdate->GetSegIdx();
    Cell *cell = segUpdate->GetCell();
    bool isConnected = segUpdate->IsConnected();

    //printf("Dequeing segment updates.\n");
    /*
     * TODO: One segment can learn to recognize multiple different patterns.
     * Currently, synapses are only added for a single pattern though.
     */
    if (!segment) {
        //printf("\tsegment is null\n");
        /*
         * A NULL segment will invoke the dequeing code to create a new segment
         * composed of synapses to cells that were active in the previous timestep in
         * order to learn a transition in the active sequence.
         */
        //printf("\tcreating new learning segment\n");
        DendriteSegment *newSeg = cell->NewSegment(this, htmPtr->FirstPattern());
        segment = newSeg;
    }

    if (segment) {
//        printf("\treinforcing existing segment\n");
        std::vector<Synapse*> synsToUpdate =
            isConnected ? segment->GetWasActiveSynapses():
                          segment->GetWasNearActiveSynapses();
            
        // synaptic modification: Hebbian learning rule
//        printf("[%d] syns to modify = %d\n", reinforce, synsToUpdate.size());
//        printf("\t\t");
        unsigned int null_syns=0;
        for (unsigned int i=0; i < synsToUpdate.size(); i++) {
            Synapse *syn = synsToUpdate[i];
//            printf("B:%f", syn->GetPerm());
            reinforce ? syn->IncPerm() : syn->DecPerm();
//            printf("A:%f ", syn->GetPerm());
            if (syn->GetPerm() <= 0)
                null_syns++;
        }
//        printf("\n");
        if (null_syns+1 == synsToUpdate.size()) {
            //printf("Deleting segment idx %d from cell 0x%08x\n",
                //segidx, cell);
            cell->RemoveSegment(segidx);
            delete segment;
        }
    }

    /*
     * After modification, remove structure from vector.
     *
     * This vector magic uses std::remove to remove the unwanted values and
     * rearrange the elements without actually changing the container size,
     * which is performed by std::vector::erase.
     */
    segmentUpdateList.erase(
        std::remove(
            segmentUpdateList.begin(),
            segmentUpdateList.end(),
            segUpdate
        ),
        segmentUpdateList.end()
    );
//    printf("Freeing segUpdate memory [0x%08x]\n", segUpdate);
    delete segUpdate;
//    printf("Done.\n");
}

void HtmSublayer::NewTimestep()
{
    Column ***columns = (Column ***)input;
    for (unsigned int i=0; i<height; i++)
        for (unsigned int j=0; j<width; j++)
            columns[i][j]->NextTimestep();
    // save number of active columns for next timestep.
    numActiveColumns[1] = numActiveColumns[0];
}

int HtmSublayer::LastActiveColumns()
{
    return numActiveColumns[1];
}

int HtmSublayer::CurrentActiveColumns()
{
    return numActiveColumns[0];
}

/*
 * Probe the neighboring columns within the dynamic inhibition radius to
 * determine whether or not the current column remains active after inhibition.
 */
bool HtmSublayer::_EligibleToFire(Column *col)
{
    // Compute neighbors within current inhibition radius.
    Column **neighbors = col->GetNeighbors(
        (Column ***)input,
        inhibitionRadius,
        (int)width, (int)height
    );
    /*
     * Compute the number of columns that can fire within the inhibition radius
     */
    int desiredLocalActivity = col->GetLocalActivity();
    if (desiredLocalActivity <= 0)
        desiredLocalActivity = 1;
    /*
     * Determine whether or not this column gets inhibited or does the
     * inhibiting.
     */
    int topOverlap = 0;
    int prevTopOverlap = 0;
    int topOverlapSet = 0;
    int firingSet = 0;
    int overlap = col->GetOverlap();
    int numNeighbors = col->GetNumNeighbors();
    /*
     * Iteratively partition neighbors into descending sets of overlap scores
     * until the number of neighbors exceeds the desiredLocalActivity. At that
     * point, the topOverlap score will be set to minLocalActivity.
     */
    while (firingSet < desiredLocalActivity) {
        /*
         * Find the next highest set of overlap scores among the neighbors
         */
        for (int i=0; i<numNeighbors; i++) {
            int neighbOverlap = neighbors[i]->GetOverlap();
            if (neighbOverlap > 0) {
                if (neighbOverlap > topOverlap && (neighbOverlap < prevTopOverlap || !prevTopOverlap)) {
                    topOverlap = neighbOverlap;
                    topOverlapSet = 1;
                } else if (neighbOverlap == topOverlap)
                    topOverlapSet++;
            }
        }
        prevTopOverlap = topOverlap;
        /*
         * Make sure it doesn't get stuck in the while loop when there aren't
         * as many active neighbors as desiredLocalActivity allows.
         */
        if (prevTopOverlap == topOverlap)
            break;
        topOverlap = 0;
        firingSet += topOverlapSet;
    }
    if (overlap >= topOverlap)
        return true;
    if (col->IsHighTierColumn())
        return true;
    return false;
}

/*
 * Computes the average connected receptive field size over all the columns. Each
 * column has access to a particular receptive field of input through its proximal
 * dendrite segment (close to cellular soma) that forms afferent synapses with a
 * subset of input cells. The receptive fields vary in the graphical ordering
 * of cells they connect to, and this results in different "fan-out" of
 * connections in the fields.
 */
void HtmSublayer::_ComputeInhibitionRadius(Column ***columns)
{
    int avg_rec_fld_rad = 0;

    for (unsigned int i=0; i<height; i++) {
        for (unsigned int j=0; j<width; j++) {
            DendriteSegment *segment = columns[i][j]->GetProximalDendriteSegment();
            std::vector<Synapse *> syns = segment->GetSynapses();
            int rfs = columns[i][j]->GetRecFieldSz();
            int s = 0;
            for (int k=0, d=0; k<rfs; k++, d=0) {
                if (syns[k]->IsConnected()) {
                    d = sqrt(
                            pow(syns[k]->GetX() - columns[i][j]->GetCenterX(), 2) +
                            pow(syns[k]->GetY() - columns[i][j]->GetCenterY(), 2)
                        );
                    s++;
                }
                avg_rec_fld_rad += d;
            }
            // just in case the column had no connected synapses...
            avg_rec_fld_rad /= s>0? s : 1;
        }
    }

    inhibitionRadius = avg_rec_fld_rad;
}

// accessors

Column*** HtmSublayer::GetInput()
{
    return (Column ***)input;
}

void HtmSublayer::setlower(GenericSublayer *reg)
{
    lower = reg;
}

void HtmSublayer::sethigher(GenericSublayer *reg)
{
    higher = reg;
}

