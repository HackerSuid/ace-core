#include <stdlib.h>

#include "pooling_layer.h"
#include "htmsublayer.h"
#include "column.h"
#include "dendritesegment.h"

PoolingLayer::PoolingLayer(
    unsigned int h, unsigned int w, unsigned int d,
    unsigned int sdrSz, Htm *htmPtr,
    HtmSublayer *inLayer
) {
    height = h;
    width = w;
    depth = d;
    this->sdrSz = sdrSz;
    numActiveInputs = 0;
    // not an smi L4 layer
    sensorimotorLayer = false;

    this->htmPtr = htmPtr;
    inputLayer = inLayer;

    input = (GenericInput ***)malloc(
        sizeof(GenericInput **) * height
    );
    for (unsigned int i=0; i<height; i++) {
        input[i] = (GenericInput **)malloc(
            sizeof(GenericInput *) * width
        );
        for (unsigned int j=0; j<width; j++) {
            input[i][j] = new Column(
                this,
                j, i, depth,
                0.02, //rec_field_sz,
                1.00, //localActivity,
                0.90, //columnComplexity,
                true, //highTier,
                100  //activityCycleWindow,
            );
        }
    }

    initBucketMap();
    offset = -1;
    num_objects = 0;
    new_object = true;

    // motor and location patterns are unused
    motorPattern = NULL;
    locationPattern = NULL;
}

PoolingLayer::~PoolingLayer()
{
    for (unsigned int i=0; i<height; i++) {
        for (unsigned int j=0; j<width; j++) {
            delete input[i][j];
        }
        delete input[i];
    }
    delete input;
}

void PoolingLayer::PoolInputColumns()
{
    Column ***columns = (Column ***)input;
    unsigned int i, j, c;

    // if there was a signal for a new object, then create a new object
    // representation.
    if (new_object) {
        unsigned int n = height*width;
        int bucketIdx = getBucketIdx(num_objects);
        std::vector<unsigned int> layerIdxs(n, 0);
        if (bucketMap.find(bucketIdx) == bucketMap.end()) {
            createNewBucket(bucketIdx);
        }
        std::vector<unsigned int> sdrColIdxs = bucketMap[bucketIdx];
        for (i=0; i<height; i++) {
            for (j=0; j<width; j++) {
                for (c=0; c<sdrColIdxs.size(); c++) {
                    if (i*height+j==sdrColIdxs[c]) {
                        columns[i][j]->ConnectToActiveInputs(inputLayer);
                        currObjRep.push_back(columns[i][j]);
                        numActiveInputs++;
                        break;
                    }
                }
            }
        }
        num_objects++;
        new_object = false;
    }

    // inference step, activate any pooled representation(s) that recognize
    // the current pattern. set others inactive.
    std::vector<Column *> inferColsAct, inferColsInact;
    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            // the dendrite of each column contains synapses to several
            // different patterns. since the input patterns are sdrs,
            // the column could subsample it, and it needn't worry about
            // mixed patterns. the code currently doesn't subsample, but
            // sdr properties still apply to the input.
            DendriteSegment *p =
                columns[i][j]->GetProximalDendriteSegment();
            unsigned int numSynsActive = 0;
            if (p) {
                numSynsActive = p->GetNumIsActiveSensorySynapses();
            }
            unsigned int currActInputs =
                (unsigned int)inputLayer->CurrentActiveColumns();
            if (numSynsActive == currActInputs)
                inferColsAct.push_back(columns[i][j]);
            else
                inferColsInact.push_back(columns[i][j]);
        }
    }
    numActiveInputs = (int)inferColsAct.size();
    for (unsigned int a=0; a<inferColsAct.size(); a++)
        inferColsAct[a]->SetActive(true, false);
    for (unsigned int ia=0; ia<inferColsInact.size(); ia++)
        inferColsInact[ia]->SetActive(false, false);

    // handle columns in obj rep that are still inactive.  pool the input
    // by adding synapses to connect the active object representation
    // to the input layer.
    printf("\tContinue this object of %u columns\n", currObjRep.size());
    for (i=0; i<currObjRep.size(); i++) {
        bool skipNewSyns = false;
        // skip columns already active after inference.
        for (unsigned int a=0; a<inferColsAct.size(); a++)
            if (currObjRep[i] == inferColsAct[a])
                skipNewSyns = true;
        if (skipNewSyns)
            continue;
        printf("\t\tconnecting col (%u,%u)\n",
            currObjRep[i]->GetY(),
            currObjRep[i]->GetX());
        currObjRep[i]->ConnectToActiveInputs(inputLayer);
        currObjRep[i]->SetActive(true, false);
    }
}

// this code is copied from location_codec
void PoolingLayer::initBucketMap()
{
    unsigned int n = height*width;
    std::vector<unsigned int> wv =
        randomRepresentation(n, sdrSz);
    bucketMap[MAX_BUCKETS/2] = wv;
}

int
PoolingLayer::getBucketIdx(unsigned int objIdx)
{
    if (offset == -1)
        offset = (int)objIdx;
    // using resolution = 1
    int idx = (MAX_BUCKETS/2) +
                       (int)(round(objIdx-offset));

    if (idx < 0)
        idx = 0;
    if (idx >= MAX_BUCKETS)
        idx = MAX_BUCKETS-1;

    return idx;
}

void PoolingLayer::createNewBucket(unsigned int idx)
{
    bucketMap[idx] = newRepresentation(idx);
}

std::vector<unsigned int>
PoolingLayer::newRepresentation(unsigned int idx)
{
    std::vector<unsigned int> redoIdx;
    bool repOK = false;
    unsigned int n = height*width;
    unsigned int w = sdrSz;

    // create a new random representation
    std::vector<unsigned int> newRep = randomRepresentation(n, w);

    // check if any bit index is already used by any other bucket
    while (repOK == false) {
        //printf("trying ");
        //for (unsigned int u=0; u<newRep.size(); u++)
        //    printf("%u ", newRep[u]);
        //printf("\n");
        //for (unsigned int i=chkMin; i<chkMax; i++) {
        for (auto bucket : bucketMap) {
            std::vector<unsigned int> buck = bucket.second;
            //printf("\tchecking vs idx %u\n", bucket.first);
            for (unsigned int j=0; j<w; j++) {
                // not worrying about adding same one twice since they
                // are unique
                for (unsigned int k=0; k<w; k++) {
                    //printf("\t\t%u v %u\n", buck[j], newRep[k]);
                    if (buck[j] == newRep[k]) {
                        redoIdx.push_back(k);
                        //printf("\t\t%u taken by %u\n", newRep[k], bucket.first);
                    }
                }
            }
        }
        for (unsigned int i=0; i<w; i++)
            for (unsigned int j=0; j<w && j!=i; j++)
                if (newRep[j] == newRep[i])
                    redoIdx.push_back(j);
        if (redoIdx.size() == 0)
            repOK = true;
        else {
            // pick new random bits to try.
            std::vector<unsigned int> rep2 = randomRepresentation(
                n, redoIdx.size()
            );
            for (unsigned k=0; k<redoIdx.size(); k++)
                newRep[redoIdx[k]] = rep2[k];
            redoIdx.clear();
            rep2.clear();
        }
    }

    return newRep;
}

std::vector<unsigned int>
PoolingLayer::randomRepresentation(
    unsigned int n, unsigned int w)
{
    std::vector<unsigned int> r = listRange(n);
    std::random_shuffle(r.begin(), r.end());
    std::vector<unsigned int> wv;
    for (unsigned int i=0;i<w;i++)
        wv.push_back(r[i]);
    return wv;
}

std::vector<unsigned int>
PoolingLayer::listRange(unsigned int n)
{
    std::vector<unsigned int> l;
    for (unsigned int i=0; i<n; i++)
        l.push_back(i);
    return l;
}

