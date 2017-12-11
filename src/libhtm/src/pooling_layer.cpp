#include <stdlib.h>

#include "pooling_layer.h"
#include "column.h"

PoolingLayer::PoolingLayer(
    unsigned int h, unsigned int w, unsigned int d,
    unsigned int maxSdrSz,
    Htm *htmPtr
) {
    height = h;
    width = w;
    depth = d;
    this->maxSdrSz = maxSdrSz;
    numActiveInputs = 0;
    // not an smi L4 layer
    sensorimotorLayer = false;

    this->htmPtr = htmPtr;

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
                0.02, //localActivity,
                0.90, //columnComplexity,
                true, //highTier,
                100, //activityCycleWindow,
                sensorimotorLayer
            );
        }
    }

    // initialize proximal dendrite, once the lower smi sublayer
    // has been fully initialized.

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

