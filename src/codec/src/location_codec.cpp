#include <ctime>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include "codec.h"
#include "location_codec.h"
#include "sensoryregion.h"
#include "sensoryinput.h"

#define MAX_BUCKETS 1000

/*
 * n denotes total number of bits in representation
 * w denotes number of active bits
 * d denotes number of subdirs in object hierarchy
 *   - d>1 not supported yet (objects are 1 subdir in depth)
 */
LocationCodec::LocationCodec(
    unsigned int n, unsigned int w, unsigned int d)
{
    this->n = n;
    this->w = w;
    this->d = d;
    minIndex = 0;
    maxIndex = 0;
    offset = 0;
    resolution = 1;

    locPattern = NULL;
    locInputs = NULL;

    std::srand(std::time(0));

    initBucketMap();
}

LocationCodec::~LocationCodec()
{
}

void LocationCodec::initBucketMap()
{
    minIndex = MAX_BUCKETS/2;
    maxIndex = MAX_BUCKETS/2;
    std::vector<unsigned int> wv = randomRepresentation(n, w);
    bucketMap[minIndex] = wv;
}

std::vector<unsigned int> LocationCodec::randomRepresentation(
    unsigned int n, unsigned int w)
{
    std::vector<unsigned int> r = listRange(n);
    std::random_shuffle(r.begin(), r.end());
    std::vector<unsigned int> wv;
    for (unsigned int i=0;i<w;i++)
        wv.push_back(r[i]);
    return wv;
}

std::vector<unsigned int> LocationCodec::listRange(unsigned int n)
{
    std::vector<unsigned int> l;
    for (unsigned int i=0; i<n; i++)
        l.push_back(i);
    return l;
}

bool LocationCodec::Init()
{
}

SensoryRegion* LocationCodec::GetPattern(int inode)
{
    unsigned int bucketIdx = getBucketIdx(inode);
    std::vector<unsigned int> output(n, 0);
    if (bucketMap.find(bucketIdx) == bucketMap.end()) {
        createNewBucket(bucketIdx);
    }// else
        //printf("Found bucket map for idx %u", bucketIdx);
    std::vector<unsigned int> bits = bucketMap[bucketIdx];
    for (unsigned int i=0;i<bits.size();i++)
        output[bits[i]] = 1;
    //printf("\t");
    //for (unsigned int o=0; o<n; o++)
        //printf("%u ", output[o]);
    //printf("\n");

    // free memory of previous pattern
    if (locPattern != NULL) {
        if (locInputs != NULL) {
            for (unsigned int i=0; i<n; i++)
                delete locInputs[0][i];
            free(locInputs[0]);
            free(locInputs);
            locInputs = NULL;
        }
        locPattern = NULL;
    }
    // SensoryRegion API uses a 2D pointer.
    SensoryInput ***locInputs = (SensoryInput ***)malloc(
        sizeof(SensoryInput **) * 1
    );
    locInputs[0] = (SensoryInput **)malloc(
        sizeof(SensoryInput *) * n
    );

    for (unsigned int i=0; i<n; i++) {
        locInputs[0][i] = new SensoryInput(i, 0);
        locInputs[0][i]->SetActive(output[i]?true:false);
        locInputs[0][i]->SetActive(output[i]?true:false);

    }

    locPattern = new SensoryRegion(
        locInputs, this->w, n, 1, 0, NULL
    );
    return locPattern;
}

unsigned int LocationCodec::getBucketIdx(int inode)
{
    if (offset == 0)
        offset = inode;
    unsigned int idx = (MAX_BUCKETS/2) +
                       (int)(round((inode-offset)/resolution));

    if (idx < 0)
        idx = 0;
    if (idx >= MAX_BUCKETS)
        idx = MAX_BUCKETS-1;

    return idx;
}

void LocationCodec::createNewBucket(unsigned int idx)
{
    bucketMap[idx] = newRepresentation(idx);
    if (idx < minIndex) {
        //if (idx == minIndex-1) {
            //bucketMap[idx] = newRepresentation(idx);
            minIndex = idx;
        //} else {
            //createNewBucket(idx+1);
            //createNewBucket(idx);
        //}
    } else {
    //    if (idx == maxIndex+1) {
    //        printf("creating bucket idx %u\n", idx);
    //        bucketMap[idx] = newRepresentation(idx);
            maxIndex = idx;
    //    } else {
    //        createNewBucket(idx-1);
    //        createNewBucket(idx);
    //    }
    }
}

std::vector<unsigned int> LocationCodec::newRepresentation(
    unsigned int idx) {
    std::vector<unsigned int> redoIdx;
    bool repOK = false;

    // create a new random representation
    std::vector<unsigned int> newRep = randomRepresentation(n, w);

    unsigned int chkMin = minIndex;
    unsigned int chkMax = maxIndex;
    if (idx>maxIndex)
        chkMax = idx;
    if (idx<minIndex)
        chkMin = idx;
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

