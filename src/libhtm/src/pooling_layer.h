#ifndef POOLING_LAYER_H_
#define POOLING_LAYER_H_

class Htm;

class PoolingLayer
{
public:
    PoolingLayer(int h, int w, Htm *htmPtr);
    ~PoolingLayer();
    int GetHeight() { return height; }
    int GetWidth() { return width; }
private:
    int height, width;
    Htm *htmPtr;
};

#endif

