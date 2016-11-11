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

Autoencoder::Autoencoder(std::map<unsigned char *, std::vector<unsigned char> > trainingPatterns)
{
    this->trainingPatterns = trainingPatterns;
    unsigned int inputDimension = ComputeByteDimension()*8;
    //printf("Autoencoder dimensions: i %u h %u o %u\n",
        //inputDimension, inputDimension/2, inputDimension);
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

    for (auto tdIter : trainingPatterns)
        if (tdIter.second.size() > highestDim)
            highestDim = tdIter.second.size();

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
        float avgErr = 0;
        for (auto tdIter : trainingPatterns) {
            std::vector<unsigned char> trainingData = tdIter.second;
            // Forward propagation through each layer.
            // input
            unsigned int inNodesSz = inNodes->size()-1;
            unsigned int fcnByteIdx=0, inod=0;
            unsigned int opcodeByte;
            while (inod < inNodesSz) {
                // insert NOPs to equalize function length with number of
                // input nodes.
                if (trainingData[fcnByteIdx]==RET_OPCODE && inod<inNodesSz-8)
                    opcodeByte = NOP_OPCODE;
                else
                    opcodeByte = trainingData[fcnByteIdx++];
                //printf("%02x ", opcodeByte);
                for (unsigned int bit=0; bit<8; bit++, inod++) {
                    (*inNodes)[inod]->SetActivationVal(
                        (float)((1<<bit & opcodeByte)>>bit)
                    );
                    //printf("%d", (int)((*inNodes)[inod]->GetActivationVal()));
                }
                //printf("\n");
            }
            //printf("\n");
            // hidden
            //printf("Feedforward hidden layer\n");
            hiddenLayer->ForwardPropagationOverNodes(false);
            // output
            //printf("Feedforward output layer\n");
            outputLayer->ForwardPropagationOverNodes(false);
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
                //printf("%f - %f = %f\n",
                    //(float)(*inNodes)[o]->GetActivationVal(),
                    //(*outNodes)[o]->GetActivationVal(),
                    //diff
                //);
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
        }
        printf("[%u] Average error: %f\n", i, avgErr/trainingPatterns.size());
        const unsigned int exportPoint =
            numEpochs*0.01>=2 ? (unsigned int)(numEpochs*0.01) : 1;
        if (i % exportPoint == 0)
            ExportNetwork();
    }
}

void Autoencoder::Classify(
    std::map<unsigned char *, std::vector<unsigned char> > pattern)
{
    std::vector<Node *> *inNodesClassify = inputLayer->GetNodes();
    std::vector<Node *> *hidNodesClassify = hiddenLayer->GetNodes();
    std::vector<Node *> *outNodesClassify = outputLayer->GetNodes();

    std::vector<Node *> *inNodesTmp = NULL;
    std::vector<Node *> *hidNodesTmp = NULL;
    std::vector<Node *> *outNodesTmp = NULL;

    unsigned int inNodesSz = inNodesClassify->size()-1;
    unsigned int fcnByteIdx=0, inod=0;
    unsigned int opcodeByte;

    unsigned char *pattName = pattern.begin()->first;
    std::vector<unsigned char> pattData = pattern.begin()->second;

    // set network input nodes to test pattern.
    while (inod < inNodesSz) {
        if (pattData[fcnByteIdx]==RET_OPCODE && inod<inNodesSz-8)
            opcodeByte = NOP_OPCODE;
        else
            opcodeByte = pattData[fcnByteIdx++];
        //printf("%02x ", opcodeByte);
        for (unsigned int bit=0; bit<8; bit++, inod++) {
            (*inNodesClassify)[inod]->SetActivationVal(
                (float)((1<<bit & opcodeByte)>>bit)
            );
            //printf("%d", (int)((*inNodesClassify)[inod]->GetActivationVal()));
        }
        //printf("\n");
    }
    hiddenLayer->ForwardPropagationOverNodes(false);
    outputLayer->ForwardPropagationOverNodes(false);

    // encode each training data pattern into input nodes.
    std::map<unsigned char *, Layer *> trainedInputs;
    for (auto tdIter : trainingPatterns) {
        std::vector<unsigned char> trainingData = tdIter.second;
        unsigned int nod = 0;
        fcnByteIdx=0;
        trainedInputs[tdIter.first] = new Layer(inNodesSz, NULL, false);
        std::vector<Node *> *trainedNodes =
            trainedInputs[tdIter.first]->GetNodes();
        while (nod < inNodesSz) {
            if (trainingData[fcnByteIdx]==RET_OPCODE && nod<inNodesSz-8)
                opcodeByte = NOP_OPCODE;
            else
                opcodeByte = trainingData[fcnByteIdx++];
            //printf("%02x ", opcodeByte);
            for (unsigned int bit=0; bit<8; bit++, nod++) {
                (*trainedNodes)[nod]->SetActivationVal(
                    (float)((1<<bit & opcodeByte)>>bit)
                );
                //printf("%d", (int)((*trainedNodes)[nod]->GetActivationVal()));
            }
            //printf("\n");
        }
    }

    float lowestErr = (float)RAND_MAX;
    unsigned char *lowPatt = NULL;
    for (auto tiIter : trainedInputs) {
        float sampErr = 0;
        std::vector<Node *> *nodes = tiIter.second->GetNodes();
        for (unsigned int o=0; o<outNodesClassify->size(); o++) {
            float diff = (*nodes)[o]->GetActivationVal() -
                         (float)(*outNodesClassify)[o]->GetActivationVal();
            //printf("%f - %f = %f\n",
                //(float)(*inNodesClassify)[o]->GetActivationVal(),
                //(*outNodesClassify)[o]->GetActivationVal(),
                //diff
            //);
            sampErr += sqrt(pow(diff, 2));
        }
        sampErr /= (*outNodesClassify).size();
        printf("\t%s error: %f\n", tiIter.first, sampErr);
        if (sampErr < lowestErr) {
            lowestErr = sampErr;
            lowPatt = tiIter.first;
        }
    }
    printf("%s is most similar to %s\n", pattName, lowPatt);
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
    unsigned int i, h;
    for (h=0; h<hidNodesSz-1; h++) {
        for (i=0; i<inNodesSz; i++) {
            float w = (*hidNodes)[h]->GetWeight((*inNodes)[i]);
            write(expfd, (const void *)&w, sizeof(float));
            //if (i<5 && h<5) printf("%f ", w);
            //write(tmpfd, (const void *)&w, sizeof(float));
        }
        //if (i==5&&h<5) printf("\n");
    }
    for (unsigned int o=0; o<outNodesSz; o++) {
        for (unsigned int h=0; h<hidNodesSz; h++) {
            float w = (*outNodes)[o]->GetWeight((*hidNodes)[h]);
            write(expfd, (const void *)&w, sizeof(float));
            //write(tmpfd, (const void *)&w, sizeof(float));
        }
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
    unsigned i, h;
    printf("importing input to hidden\n");
    for (h=0; h<hidNodesSz-1; h++) {
        for (i=0; i<inNodesSz; i++) {
            read(impfd, (void *)&w, sizeof(float));
            //if (i<5 && h<5) printf("%f ", w);
            //write(tmpfd, (void *)&w, sizeof(float));
            (*hidNodes)[h]->SetWeight((*inNodes)[i], w);
        }
        //if (i==5&&h<5) printf("\n");
    }
    for (unsigned int o=0; o<outNodesSz; o++) {
        for (unsigned int h=0; h<hidNodesSz; h++) {
            read(impfd, (void *)&w, sizeof(float));
            //write(tmpfd, (void *)&w, sizeof(float));
            (*outNodes)[o]->SetWeight((*hidNodes)[h], w);
        }
    }

    close(impfd);
    //close(tmpfd);
}

