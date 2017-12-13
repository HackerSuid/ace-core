#ifndef POOLING_LAYER_H_
#define POOLING_LAYER_H_

#include <algorithm>
#include <vector>
#include <map>
#include "genericsublayer.h"

#define MAX_BUCKETS 1000

class Htm;
class HtmSublayer;

class PoolingLayer : public GenericSublayer
{
public:
    PoolingLayer(
        unsigned int h, unsigned int w, unsigned int d,
        unsigned int sdrSz, Htm *htmPtr,
        HtmSublayer *inLayer
    );
    ~PoolingLayer();
    void InitializeProximalDendrites();
    void PoolInputColumns();

    void initBucketMap();
    int getBucketIdx(unsigned int objIdx);
    void createNewBucket(unsigned int idx);
    std::vector<unsigned int> newRepresentation(
        unsigned int idx);
    std::vector<unsigned int> randomRepresentation(
        unsigned int n, unsigned int w
    );
    std::vector<unsigned int> listRange(
        unsigned int n
    );

    int GetHeight() { return height; }
    int GetWidth() { return width; }
private:
    unsigned int height, width, depth;
    unsigned int sdrSz;
    unsigned int num_objects;
    Htm *htmPtr;
    HtmSublayer *inputLayer;
    std::map<unsigned int, std::vector<unsigned int> >
        bucketMap;
    int offset;
};

#endif

