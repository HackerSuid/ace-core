#include <vector>
#include <list>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "Autoencoder.h"

#define DEF_EPOCHS  50000

std::vector< std::vector<int> > trainingData;

int main(int argc, char **argv)
{
    unsigned int numEpochs = DEF_EPOCHS;

    if (argc > 1)
        numEpochs = (unsigned int)atoi(argv[1]);
    if (numEpochs < DEF_EPOCHS)
        printf("Number of epochs may be insufficient.\n");

    srand(time(NULL)+getpid());

/*
    trainingData.push_back({0, 0});
    trainingData.push_back({0, 1});
    trainingData.push_back({1, 0});
    trainingData.push_back({1, 1});
*/

    trainingData.push_back({
        1, 1, 0, 1, 0, 0, 0, 1,
        0, 0, 1, 1, 1, 0, 0, 0,
        0, 0, 1, 0, 0, 1, 0, 0,
        1, 1, 0, 0, 0, 0, 1, 1
    });
    trainingData.push_back({
        1, 1, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 0, 0, 1,
        0, 0, 0, 0, 1, 0, 0, 1,
        1, 1, 0, 0, 0, 0, 1, 1
    });

    unsigned int inputDimension = trainingData[0].size();

    Autoencoder *ae = new Autoencoder(inputDimension);
    ae->Train(numEpochs, trainingData);

    return 0;
}

