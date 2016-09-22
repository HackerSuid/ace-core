#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "Layer.h"
#include "Node.h"

Node::Node(Layer *inputLayer, bool isBiasNode)
{
    // if this is not the first layer or a bias node, initialize
    // weighted connections to input nodes.
    inLayer = inputLayer;
    if (inputLayer && !isBiasNode) {
        std::vector<Node *> *inputNodes = inputLayer->GetNodes();
        printf("\t");
        for (unsigned int i=0; i<inputNodes->size(); i++) {
            synapseLinks[(*inputNodes)[i]] =
                //0.40+((rand()%(int)(RAND_MAX*0.20))/(float)RAND_MAX);
                (rand()%2 ? rand() : -1.0*rand())/(float)RAND_MAX;
            printf("%f ", synapseLinks[(*inputNodes)[i]]);
        }
        printf("\n");
    }
    this->isBiasNode = isBiasNode;
    if (isBiasNode)
        activationVal = 1.0;
}

Node::~Node()
{
}

void Node::ComputeActivation()
{
    // activation of bias nodes remains at the initialized constant
    // of 1.
    if (isBiasNode)
        return;

    std::vector<Node *> *inNodes = inLayer->GetNodes();
    unsigned int numInNodes = inNodes->size();
    linearWeightedSummation = 0;

    printf("\tweighted sum: logistic(");
    for (unsigned int i=0; i<numInNodes; i++) {
        printf("%f*%f%s", (*inNodes)[i]->GetActivationVal(),
                          synapseLinks[(*inNodes)[i]],
                          i==numInNodes-1?"":"+"
        );
        linearWeightedSummation +=
            (*inNodes)[i]->GetActivationVal() *
            synapseLinks[(*inNodes)[i]];
    }
    activationVal = logistic(linearWeightedSummation);
    printf(")=%f\n", activationVal);
}

void Node::BackPropagateDelta()
{
    if (isBiasNode)
        return;

    float inActivationVal;
    std::vector<Node *> *inNodes = inLayer->GetNodes();

    for (unsigned int i=0; i<inNodes->size(); i++) {
        inActivationVal = (*inNodes)[i]->GetActivationVal();
        //printf("\t\t%f - %f * %f * %f = ",
        //    synapseLinks[(*inNodes)[i]],
        //    LRATE, nodeDelta, inActivationVal
        //);
        synapseLinks[(*inNodes)[i]] -=
            LRATE*nodeDelta*inActivationVal;
        //printf("%f\n", synapseLinks[(*inNodes)[i]]);
    }
}

// Compute the delta for an output node.
void Node::ComputeNodeDelta(float target)
{
    nodeDelta = (-1.0*(target - activationVal)) *
                logisticPrime(linearWeightedSummation);
    printf("\t%f = (-1.0*(%f - %f)) * %f\n",
        nodeDelta, target, activationVal,
        logisticPrime(linearWeightedSummation));
    printf("\t\t%f * (1 - %f) = %f\n",
        logistic(linearWeightedSummation),
        logistic(linearWeightedSummation),
        logisticPrime(linearWeightedSummation));
}

// Compute the delta for a hidden node.
void Node::ComputeNodeDelta(std::vector<Node *> outNodes)
{
    nodeDelta = 0;

    for (unsigned int i=0; i<outNodes.size(); i++) {
        nodeDelta += outNodes[i]->GetNodeDelta() *
                     outNodes[i]->GetWeight(this);
    }
    nodeDelta *= logisticPrime(linearWeightedSummation);
}

