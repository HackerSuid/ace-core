#ifndef AUTOENCODER_H_
#define AUTOENCODER_H_

#include <vector>
#include <list>

class Layer;

class Autoencoder
{
public:
    Autoencoder(unsigned int inputDimension);
    ~Autoencoder();
    void Train(
        unsigned int numEpochs,
        std::vector< std::vector<int> > trainingData
    );
private:
    Layer *inputLayer, *hiddenLayer, *outputLayer;
};

#endif

