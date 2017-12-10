#include "pooling_layer.h"

PoolingLayer::PoolingLayer(
    int h, int w,
    Htm *htmPtr
) {
    height = h;
    width = w;
    this->htmPtr = htmPtr;
}

PoolingLayer::~PoolingLayer()
{
}

