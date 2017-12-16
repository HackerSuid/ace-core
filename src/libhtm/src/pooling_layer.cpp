#include <stdlib.h>

#include "pooling_layer.h"
#include "htmsublayer.h"
#include "column.h"

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
    currCols = NULL;
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
                100, //activityCycleWindow,
                sensorimotorLayer
            );
        }
    }

    initBucketMap();
    offset = -1;
    num_objects = 0;

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

    if ((unsigned int)numActiveInputs != sdrSz) {
        // object was reset, so create new object representation.
        unsigned int n = height*width;
        int bucketIdx = getBucketIdx(num_objects);
        std::vector<unsigned int> layerIdxs(n, 0);
        if (bucketMap.find(bucketIdx) == bucketMap.end()) {
           createNewBucket(bucketIdx);
        }
        std::vector<unsigned int> sdrColIdxs = bucketMap[bucketIdx];
        bool colActive = false;
        for (i=0; i<height; i++) {
            for (j=0; j<width; j++) {
                for (c=0; c<sdrColIdxs.size(); c++) {
                    if (i*height+j==sdrColIdxs[c]) {
                        columns[i][j]->SetActive(true, false);
                        columns[i][j]->ConnectToActiveInputs(inputLayer);
                        colActive = true;
                        numActiveInputs++;
                        break;
                    }
                }
                if (!colActive)
                    columns[i][j]->SetActive(false, false);
            }
        }
        num_objects++;
        currCols = new std::vector<unsigned int>(sdrColIdxs);
    } else {
        // continue learning current object by reinforcing the
        // currently active synapses.
        for (i=0; i<currCols->size(); i++) {
            unsigned int x = (*currCols)[i]%width;
            unsigned int y = (*currCols)[i]/height;
            if (columns[y][x]->IsActive())
                columns[y][x]->ConnectToActiveInputs(inputLayer);
        }
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
PoolingLayer::newRepresentation(unsigned int idx) {
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

