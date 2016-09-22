#include <vector>
#include <list>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "Autoencoder.h"

std::vector< std::vector< std::vector<int> > > trainingData;

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Provide number of epochs to train.\n");
        abort();
    }

    //trainingData.push_back({{0, 0}, {0}});
    trainingData.push_back({{0, 1}, {1}});
    trainingData.push_back({{1, 0}, {1}});
    trainingData.push_back({{1, 1}, {0}});

    unsigned int numInNodes = trainingData[0][0].size();

    srand(time(NULL)+getpid());

    //Autoencoder *ae = new Autoencoder(numInNodes, numInNodes/2);
    Autoencoder *ae = new Autoencoder(numInNodes, numInNodes);

    ae->Train(atoi(argv[1]), trainingData);

    return 0;
}

