#ifndef LAYER_H_
#define LAYER_H_

#include <vector>

class Node;

class Layer
{
public:
    Layer(unsigned int numNodes, Layer *lower, bool addBias);
    ~Layer();
    void ForwardPropagationOverNodes(bool debugPrint);
    void BackwardPropagationOverNodes();
    std::vector<Node *>* GetNodes() { return nodes; }
private:
    Layer *inputLayer;
    std::vector<Node *> *nodes;
};

#endif

