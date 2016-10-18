#ifndef NODE_H_
#define NODE_H_

#include <cmath>
#include <vector>
#include <map>

#define LRATE 0.8

class Layer;

class Node
{
public:
    Node(Layer *inputLayer, bool isBiasNode);
    ~Node();
    void ComputeActivation();
    void BackPropagateDelta();
    // output node delta
    void ComputeNodeDelta(float totalError);
    // hidden node delta
    void ComputeNodeDelta(std::vector<Node *> outNodes);
    float GetActivationVal() { return activationVal; }
    float GetNodeDelta() { return nodeDelta; }
    float GetWeight(Node *node)
    {
        return synapseLinks[node];
    }
    void SetWeight(Node *node, float w)
    {
        synapseLinks[node] = w;
    }
    // used on the input layer.
    void SetActivationVal(float v) { activationVal = v; }
private:
    float logistic(float x)
    {
        return 1.0/(1.0 + exp(-1.0*x));
    }
    float logisticPrime(float x)
    {
        return logistic(x)*(1 - logistic(x));
    }

    Layer *inLayer;
    std::map<Node *, float> synapseLinks;
    float linearWeightedSummation;
    float activationVal;
    float nodeDelta;
    bool isBiasNode;
};

#endif

