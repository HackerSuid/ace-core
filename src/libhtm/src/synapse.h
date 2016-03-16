#ifndef SYNAPSE_H_
#define SYNAPSE_H_

#include <vector>

#define CONNECTED_PERM  0.20
#define PERM_INC        0.15
#define PERM_DEC        0.025
#define NEAR_CONNECTED  CONNECTED_PERM-(CONNECTED_PERM-0.05)
//#define NEAR_CONNECTED CONNECTED_PERM-PERM_DEC

class GenericInput;
class GenericRegion;

class Synapse
{
private:
    GenericInput *source;
    int srcx, srcy;
    float perm;
public:
    Synapse(GenericInput *src, int x, int y);
    ~Synapse();
    void RefreshSynapse(GenericRegion *NewPattern);
    bool IsFiring();
    bool WasFiring();
    bool IsLearning();
    bool WasLearning();
    bool IsConnected();
    bool IsNearConnected();
    float GetPerm();
    void IncPerm(float factor=1.0);
    void DecPerm(float factor=1.0);
    int GetX();
    int GetY();
    GenericInput* GetSource();
};

#endif

