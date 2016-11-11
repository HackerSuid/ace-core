#ifndef AUTOENCODER_H_
#define AUTOENCODER_H_

#include <vector>
#include <list>

#include "codec.h"

#define EXPORT_NET_FILE   "netdmp.bin"

class Layer;

class Autoencoder
{
public:
    Autoencoder(std::map<unsigned char *, std::vector<unsigned char> > trainingPatterns);
    ~Autoencoder();
    unsigned int ComputeByteDimension();
    void Train(unsigned int numEpochs);
    void Classify(std::map<unsigned char *, std::vector<unsigned char> > pattern);
    void ExportNetwork();
    void ImportNetwork();
private:
    Layer *inputLayer, *hiddenLayer, *outputLayer;
    std::map<unsigned char *, std::vector<unsigned char> > trainingPatterns;
};

#endif

