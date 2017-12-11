#ifndef POOLING_LAYER_H_
#define POOLING_LAYER_H_

#include "genericsublayer.h"

class Htm;

class PoolingLayer : public GenericSublayer
{
public:
    PoolingLayer(
        unsigned int h, unsigned int w, unsigned int d,
        unsigned int maxSdrSz, Htm *htmPtr
    );
    ~PoolingLayer();
    int GetHeight() { return height; }
    int GetWidth() { return width; }
private:
    unsigned int height, width, depth;
    unsigned int maxSdrSz;
    Htm *htmPtr;
};

#endif

