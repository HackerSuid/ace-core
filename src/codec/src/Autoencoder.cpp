#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Autoencoder.h"
#include "Layer.h"
#include "Node.h"

Autoencoder::Autoencoder(std::vector< std::vector<unsigned char> > trainingData)
{
    this->trainingData = trainingData;
    unsigned int inputDimension = ComputeByteDimension()*8;
    printf("Autoencoder dimensions: i %u h %u o %u\n",
        inputDimension, inputDimension/2, inputDimension);
    //printf("creating input nodes\n");
    inputLayer = new Layer(inputDimension, NULL, true);
    //printf("creating hidden nodes\n");
    hiddenLayer = new Layer(inputDimension/2, inputLayer, true);
    //printf("creating output nodes\n");
    outputLayer = new Layer(inputDimension, hiddenLayer, false);
}

Autoencoder::~Autoencoder()
{
    
}

unsigned int Autoencoder::ComputeByteDimension()
{
    unsigned int highestDim = 0;

    for (unsigned int i=0; i<trainingData.size(); i++)
        if (trainingData[i].size() > highestDim)
            highestDim = trainingData[i].size();

    return highestDim;
}

void Autoencoder::Train(
    unsigned int numEpochs)
{
    std::vector<Node *> *inNodes = inputLayer->GetNodes();
    std::vector<Node *> *hidNodes = hiddenLayer->GetNodes();
    std::vector<Node *> *outNodes = outputLayer->GetNodes();

    // train over every example for a specified number of epochs.
    for (unsigned int i=0; i<numEpochs; i++) {
        printf("epoch %u\n", i);
        float avgErr = 0;
        for (unsigned int e=0; e<trainingData.size(); e++) {
            //printf("\texample %u/%u\n", e+1, trainingData.size());
            // Forward propagation through each layer.
            // input
            //printf("example is %u bytes, inNode is %u bytes\n",
                //trainingData[e].size(), inNodes->size());

            unsigned int inNodesSz = inNodes->size()-1;
            unsigned int fcnByteIdx, opcodeByte;
            for (unsigned int inod=0, fcnByteIdx=0; inod<inNodesSz; inod++) {
                // insert NOPs to equalize function length with number of
                // input nodes.
                if (trainingData[e][fcnByteIdx]==RET_OPCODE && inod<inNodesSz-1)
                    opcodeByte = NOP_OPCODE;
                else
                    opcodeByte = trainingData[e][fcnByteIdx++];
                //printf("%02x ", opcodeByte);
                for (unsigned int bit=0; bit<8; bit++) {
                    //printf("%d", (1<<bit & trainingData[e][inod])>>bit);
                    (*inNodes)[inod]->SetActivationVal(
                        (float)((1<<bit & opcodeByte)>>bit)
                    );
                }
                //printf("\n");
            }
            //printf("\n");
            // hidden
            //printf("Feedforward hidden layer\n");
            hiddenLayer->ForwardPropagationOverNodes();
            // output
            //printf("Feedforward output layer\n");
            outputLayer->ForwardPropagationOverNodes();

            //if (i == numEpochs-1) {
            //    printf("target\tactual\n");
            //    for (unsigned int in=0; in<inNodes->size(); i++)
            //        printf("%d\t%f\n",
            //            trainingData[e][in],
            //            (*outNodes)[in]->GetActivationVal()
            //        );
            //}

            // Compute overall loss function error
            float sampErr = 0;
            for (unsigned int o=0; o<outNodes->size(); o++) {
                //printf("%f - %f = %f\n",
                    //(*outNodes)[o]->GetActivationVal(), (float)trainingData[e][o],
                    //(*outNodes)[o]->GetActivationVal()-(float)trainingData[e][o]
                //);
                sampErr += (*outNodes)[o]->GetActivationVal() -
                          (float)trainingData[e][o];
            }
            sampErr /= (*outNodes).size();
            avgErr += sampErr;

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
        printf("[%u] Average error: %f\n", i, avgErr/trainingData.size());
    }
}

