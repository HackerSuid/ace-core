#ifndef AUTOENCODER_H_
#define AUTOENCODER_H_

#include <vector>
#include <list>

#include "codec.h"

class Layer;

class Autoencoder
{
public:
    Autoencoder(std::vector< std::vector<unsigned char> > trainingData);
    ~Autoencoder();
    unsigned int ComputeByteDimension();
    void Train(unsigned int numEpochs);
private:
    Layer *inputLayer, *hiddenLayer, *outputLayer;
    std::vector< std::vector<unsigned char> > trainingData;
};

#endif

