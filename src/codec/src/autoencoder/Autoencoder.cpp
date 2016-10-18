#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Autoencoder.h"
#include "Layer.h"
#include "Node.h"

Autoencoder::Autoencoder(unsigned int inputDimension)
{
    printf("Autoencoder dimensions: i %u h %u o %u\n",
        inputDimension, inputDimension/2, inputDimension);
    //printf("creating input nodes\n");
    inputLayer = new Layer(inputDimension, NULL, true);
    //printf("creating hidden nodes\n");
    hiddenLayer = new Layer(inputDimension, inputLayer, true);
    //printf("creating output nodes\n");
    outputLayer = new Layer(inputDimension, hiddenLayer, false);
}

Autoencoder::~Autoencoder()
{
}

void Autoencoder::Train(
    unsigned int numEpochs,
    std::vector< std::vector<int> > trainingData)
{
    std::vector<Node *> *inNodes = inputLayer->GetNodes();
    std::vector<Node *> *hidNodes = hiddenLayer->GetNodes();
    std::vector<Node *> *outNodes = outputLayer->GetNodes();

    // train over every example for a specified number of epochs.
    for (unsigned int i=0; i<numEpochs; i++) {
        float avgErr = 0;
        for (unsigned int e=0; e<trainingData.size(); e++) {
            /*if (i == numEpochs-1) {
                printf("training data: ");
                printf("\t%d %d => %d %d\n",
                    trainingData[e][0],
                    trainingData[e][1],
                    trainingData[e][0],
                    trainingData[e][1]
                );
            }*/
            // Forward propagation through each layer.
            // input
            for (unsigned int inod=0; inod<inNodes->size()-1; inod++)
                (*inNodes)[inod]->SetActivationVal(
                    (float)trainingData[e][inod]
                );
            // hidden
            //printf("Feedforward hidden layer\n");
            hiddenLayer->ForwardPropagationOverNodes();
            // output
            //printf("Feedforward output layer\n");
            outputLayer->ForwardPropagationOverNodes();

            if (i == numEpochs-1) {
                float sampErr = 0;
                for (unsigned j=0; j<outNodes->size(); j++) {
                    float diff = (float)trainingData[e][j] -
                                 (*outNodes)[j]->GetActivationVal();
                    sampErr += sqrt(pow(diff, 2));
                    //printf("target %d, actual %f\n",
                    //    trainingData[e][j],
                    //    (*outNodes)[j]->GetActivationVal()
                    //);
                }
                sampErr /= (*outNodes).size();
                avgErr += sampErr;
            }

            // Compute output layer deltas.
            //printf("output layer deltas\n");
            for (unsigned int j=0; j<outNodes->size(); j++)
                (*outNodes)[j]->ComputeNodeDelta(
                    (float)trainingData[e][j]
                );
            //printf("\tout node delta %f\n",
            //    (*outNodes)[0]->GetNodeDelta()
            //);

            // Compute hidden layer deltas.
            //printf("hidden layer deltas\n");
            for (unsigned int j=0; j<hidNodes->size()-1; j++)
                (*hidNodes)[j]->ComputeNodeDelta(*outNodes);
            //printf("\thid node delta %f\n",
            //    (*outNodes)[0]->GetNodeDelta()
            //);
            
            // Backward propagation of the deltas to update weights.
            //printf("Backprop:\n");
            //printf("\toutput layer\n");
            outputLayer->BackwardPropagationOverNodes();
            //printf("\thidden layer\n");
            hiddenLayer->BackwardPropagationOverNodes();
            //printf("\tout node weight/bias %f %f\n",
            //    (*outNodes)[0]->GetWeight((*hidNodes)[0]),
            //    (*outNodes)[0]->GetWeight((*hidNodes)[1])
            //);
            //printf("\thid node weight/bias %f %f\n",
            //    (*hidNodes)[0]->GetWeight((*inNodes)[0]),
            //    (*hidNodes)[0]->GetWeight((*inNodes)[1])
            //);

            //for (unsigned j=0; j<outNodes->size(); j++) {
            //    printf("node %u: %f\n", j, (*outNodes)[j]->GetActivationVal());
            //}
        }
        if (i==numEpochs-1)
            printf("Avg err = %f\n", avgErr/trainingData.size());
        //printf("\n");
    }
}

