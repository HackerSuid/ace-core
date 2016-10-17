#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
    hiddenLayer = new Layer(inputDimension, inputLayer, true);
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

    // Import weight values from exported network file.
    ImportNetwork();

    // train over every example for a specified number of epochs.
    for (unsigned int i=0; i<numEpochs; i++) {
        //printf("epoch %u\n", i);
        //if (i==1) return;
        float avgErr = 0;
        for (unsigned int e=0; e<trainingData.size(); e++) {
            //printf("\texample %u/%u\n", e+1, trainingData.size());
            // Forward propagation through each layer.
            // input
            //printf("example is %u bytes, inNode is %u bytes\n",
                //trainingData[e].size(), inNodes->size());

            unsigned int inNodesSz = inNodes->size()-1;
            unsigned int fcnByteIdx=0, inod=0;
            unsigned int opcodeByte;
            while (inod < inNodesSz) {
                // insert NOPs to equalize function length with number of
                // input nodes.
                if (trainingData[e][fcnByteIdx]==RET_OPCODE && inod<inNodesSz-8)
                    opcodeByte = NOP_OPCODE;
                else
                    opcodeByte = trainingData[e][fcnByteIdx++];
                //printf("%02x ", opcodeByte);
                for (unsigned int bit=0; bit<8; bit++, inod++) {
                    (*inNodes)[inod]->SetActivationVal(
                        (float)((1<<bit & opcodeByte)>>bit)
                    );
                    //printf("%d", (int)((*inNodes)[inod]->GetActivationVal()));
                }
                //printf("\n");
            }
            //for (unsigned int p=0; p<inNodes->size(); p++)
                //printf("%f ", (*inNodes)[p]->GetActivationVal());
            //printf("\n");
            //if (e==trainingData.size()-1)
                //return;
            // hidden
            //printf("Feedforward hidden layer\n");
            hiddenLayer->ForwardPropagationOverNodes();
            //ExportNetwork();
            //return;
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
                float diff = (float)(*inNodes)[o]->GetActivationVal() -
                             (*outNodes)[o]->GetActivationVal();
                printf("%f - %f = %f\n",
                    (float)(*inNodes)[o]->GetActivationVal(),
                    (*outNodes)[o]->GetActivationVal(),
                    diff
                );
                sampErr += sqrt(pow(diff, 2));
            }
            sampErr /= (*outNodes).size();
            avgErr += sampErr;

            // Compute output layer deltas.
            //printf("output layer deltas\n");
            unsigned int onod = 0;
            while (onod < inNodesSz) {
                (*outNodes)[onod]->ComputeNodeDelta(
                    (*inNodes)[onod]->GetActivationVal()
                );
                onod++;
            }
            /*for (unsigned int j=0; j<oNodesSz; j++)
                (*outNodes)[j]->ComputeNodeDelta(
                    (float)trainingData[e][j]
                );*/
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
        ExportNetwork();
    }
}

void Autoencoder::ExportNetwork()
{
    std::vector<Node *> *inNodes = inputLayer->GetNodes();
    std::vector<Node *> *hidNodes = hiddenLayer->GetNodes();
    std::vector<Node *> *outNodes = outputLayer->GetNodes();
    unsigned int inNodesSz = (*inNodes).size();
    unsigned int hidNodesSz = (*hidNodes).size();
    unsigned int outNodesSz = (*outNodes).size();

    int expfd = open(EXPORT_NET_FILE, O_RDWR|O_CREAT|O_TRUNC);
    //int tmpfd = open("exported", O_RDWR|O_CREAT|O_TRUNC);

    if (expfd < 0) {
        perror("open failed");
        return;
    }

    printf("exporting input to hidden\n");
    for (unsigned int h=0; h<hidNodesSz-1; h++)
        for (unsigned int i=0; i<inNodesSz; i++) {
            float w = (*hidNodes)[h]->GetWeight((*inNodes)[i]);
            write(expfd, (const void *)&w, sizeof(float));
            //write(tmpfd, (const void *)&w, sizeof(float));
        }
    for (unsigned int o=0; o<outNodesSz; o++)
        for (unsigned int h=0; h<hidNodesSz; h++) {
            float w = (*outNodes)[o]->GetWeight((*hidNodes)[h]);
            write(expfd, (const void *)&w, sizeof(float));
            //write(tmpfd, (const void *)&w, sizeof(float));
        }

    close(expfd);
    //close(tmpfd);
}

void Autoencoder::ImportNetwork()
{
    std::vector<Node *> *inNodes = inputLayer->GetNodes();
    std::vector<Node *> *hidNodes = hiddenLayer->GetNodes();
    std::vector<Node *> *outNodes = outputLayer->GetNodes();
    unsigned int inNodesSz = (*inNodes).size();
    unsigned int hidNodesSz = (*hidNodes).size();
    unsigned int outNodesSz = (*outNodes).size();

    int impfd = open(EXPORT_NET_FILE, O_RDONLY);
   // int tmpfd = open("imported", O_RDWR|O_CREAT|O_TRUNC);

    if (impfd < 0) {
        perror("open failed");
        return;
    }

    float w;
    printf("importing input to hidden\n");
    for (unsigned int h=0; h<hidNodesSz-1; h++)
        for (unsigned int i=0; i<inNodesSz; i++) {
            read(impfd, (void *)&w, sizeof(float));
            //write(tmpfd, (void *)&w, sizeof(float));
            (*hidNodes)[h]->SetWeight((*inNodes)[i], w);
        }
    for (unsigned int o=0; o<outNodesSz; o++)
        for (unsigned int h=0; h<hidNodesSz; h++) {
            read(impfd, (void *)&w, sizeof(float));
            //write(tmpfd, (void *)&w, sizeof(float));
            (*outNodes)[o]->SetWeight((*hidNodes)[h], w);
        }

    close(impfd);
    //close(tmpfd);
}

