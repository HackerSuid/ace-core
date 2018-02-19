#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cell.h"
#include "synapse.h"
#include "genericsublayer.h"
#include "genericinput.h"

Synapse::Synapse(GenericInput *src, int x, int y, input_t synType)
{
    source = src;
    type = synType;
    srcx = x;
    srcy = y;
    // synaptic permanence value is initialized around the connected threshold.
    //perm = CONNECTED_PERM+(rand()>(RAND_MAX/2)?-1:1)*(((float)rand()/RAND_MAX)*CONNECTED_PERM);
    perm = CONNECTED_PERM;
}

Synapse::~Synapse()
{
}

void Synapse::RefreshSynapse(GenericSublayer *NewPattern)
{
    if (type == SENSORY_PROXIMAL) {
        source = (NewPattern->GetInput())[srcy][srcx];
    }
    // srcx & srcy correspond to the unit grid positions
    // instead of where it is in the input pattern.
    if (type == LOCATION_DISTAL) {
        unsigned int sqw = (unsigned int)sqrt(
            NewPattern->GetWidth()
        );
        unsigned int pos = (unsigned int)(srcy*sqw)+srcx;
        source =
            (NewPattern->GetInput())[0][pos];
    }
}

bool Synapse::IsFiring()
{
    if (source->IsActive() && perm >= CONNECTED_PERM)
        return true;
    return false;
}

bool Synapse::WasFiring()
{
    if (source->WasActive() && perm >= CONNECTED_PERM)
        return true;
    return false;
}

bool Synapse::IsLearning()
{
    return ((Cell *)source)->IsLearning();
}

bool Synapse::WasLearning()
{
    return ((Cell *)source)->WasLearning();
}

bool Synapse::IsConnected()
{
    return perm >= CONNECTED_PERM ? true : false;
}

bool Synapse::IsNearConnected()
{
    return perm >= NEAR_CONNECTED ? true : false;
}

float Synapse::GetPerm()
{
    return perm;
}

// 1.0 is the ceiling
void Synapse::IncPerm(float factor)
{
    float delta = PERM_INC*factor;
    perm = (perm+delta)>1.0 ? 1.0 : perm+delta;
}

// 0.0 is the floor
void Synapse::DecPerm(float factor)
{
    float delta = PERM_DEC*factor;
    perm = (perm-delta)<0.0 ? 0.0 : perm-delta;
}

int Synapse::GetX()
{
    return srcx;
}

int Synapse::GetY()
{
    return srcy;
}

GenericInput* Synapse::GetSource()
{
    return source;
}

