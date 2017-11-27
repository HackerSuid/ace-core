#ifndef SYNAPSE_H_
#define SYNAPSE_H_

#include <vector>

#include "dendritesegment.h"

#define CONNECTED_PERM  0.200
#define PERM_INC        0.150
#define PERM_DEC        0.100
#define NEAR_CONNECTED  CONNECTED_PERM-(CONNECTED_PERM-0.05)
//#define NEAR_CONNECTED CONNECTED_PERM-PERM_DEC

class GenericInput;
class GenericSublayer;

class Synapse
{
private:
    GenericInput *source;
    input_t type;
    int srcx, srcy;
    float perm;
public:
    Synapse(GenericInput *src, int x, int y, input_t synType);
    ~Synapse();
    void RefreshSynapse(GenericSublayer *NewPattern);
    bool IsFiring();
    bool WasFiring();
    bool IsLearning();
    bool WasLearning();
    bool IsConnected();
    bool IsNearConnected();
    bool IsMotor() { return type==MOTOR_DISTAL; }
    bool IsLateral() { return type==LATERAL_DISTAL; }
    bool IsSensory() { return type==SENSORY_DISTAL; }
    bool IsLocation() { return type==LOCATION_DISTAL; }
    input_t GetType() { return type; }
    float GetPerm();
    void IncPerm(float factor=1.0);
    void DecPerm(float factor=1.0);
    int GetX();
    int GetY();
    GenericInput* GetSource();
};

#endif

