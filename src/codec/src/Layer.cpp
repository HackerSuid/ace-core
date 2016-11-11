#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Layer.h"
#include "Node.h"

Layer::Layer(unsigned int numNodes, Layer *lower, bool addBias)
{
    inputLayer = lower;
    nodes = new std::vector<Node *>();
    // create the nodes for this layer.
    for (unsigned int i=0; i<numNodes; i++)
        nodes->push_back(new Node(lower, false));
    // add a bias node to the layer.
    if (addBias)
        nodes->push_back(new Node(lower, true));
}

Layer::~Layer()
{
}

void Layer::ForwardPropagationOverNodes(bool debugPrint)
{
    unsigned int numNodes = nodes->size();

    for (unsigned int i=0; i<numNodes; i++) {
        ((*nodes)[i])->ComputeActivation(debugPrint);
    }
}

void Layer::BackwardPropagationOverNodes()
{
    unsigned int numNodes = nodes->size();

    for (unsigned int i=0; i<numNodes; i++)
        ((*nodes)[i])->BackPropagateDelta();
}

