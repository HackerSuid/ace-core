#include <cstddef>
#include <stdlib.h>
#include <stdio.h>

#include "dendritesegment.h"
#include "synapse.h"
#include "genericinput.h"

DendriteSegment::DendriteSegment()
{
    numSynapses = 0;
}

DendriteSegment::~DendriteSegment()
{
}

bool DendriteSegment::IsActive()
{
    int activeSyns = GetNumIsActiveSynapses();
    if (activeSyns and (activeSyns >= (int)numSynapses*SUBSAMPLE_THRESHOLD))
        return true;
    return false;
}

bool DendriteSegment::IsActiveFromLearning()
{
    if (GetNumIsLearningSynapses() >= (int)numSynapses*SUBSAMPLE_THRESHOLD)
        return true;
    return false;
}

void DendriteSegment::NewSynapse(Synapse *newSyn)
{
    synapses.push_back(newSyn);
    numSynapses++;
}

void DendriteSegment::RefreshSynapses(GenericRegion *NewPattern)
{
    for (int i=0; i<numSynapses; i++)
        synapses[i]->RefreshSynapse(NewPattern);
}

std::vector<Synapse *> DendriteSegment::GetSynapses()
{
    return synapses;
}

int DendriteSegment::GetNumSynapses()
{
    return synapses.size();
}

/*
 * returns the synapses that are active on the dendrite segment during the
 * current timestep. the "current" timestep is determined by the context of
 * the calling function.
 */
std::vector<Synapse*> DendriteSegment::GetIsActiveSynapses()
{
    std::vector<Synapse*> activeSyns;

    for (int i=0; i<numSynapses; i++)
        if (synapses[i]->IsFiring())
            activeSyns.push_back(synapses[i]);

    return activeSyns;
}

/* Same as above but for the previous timestep */
std::vector<Synapse*> DendriteSegment::GetWasActiveSynapses()
{
    std::vector<Synapse*> activeSyns;

    for (int i=0; i<numSynapses; i++)
        if (synapses[i]->WasFiring())
            activeSyns.push_back(synapses[i]);

    return activeSyns;
}

std::vector<Synapse*> DendriteSegment::GetWasNearActiveSynapses()
{
    std::vector<Synapse*> nearActiveSyns;

    for (int i=0; i<numSynapses; i++)
        if (synapses[i]->IsNearConnected() && synapses[i]->GetSource()->WasActive())
            nearActiveSyns.push_back(synapses[i]);

    return nearActiveSyns;
}

/*
 * returns the number of active synapses as defined above.
 */
int DendriteSegment::GetNumIsActiveSynapses()
{
    int activeSyns = 0;

    for (int i=0; i<numSynapses; i++)
        if (synapses[i]->IsFiring())
            activeSyns++;

    return activeSyns;
}

/*
 * Returns the number of synapses that are nearly connected and active.
 */
int DendriteSegment::GetNumIsNearActiveSynapses()
{
    int nearActiveSyns = 0;

    for (int i=0; i<numSynapses; i++)
        if (synapses[i]->IsNearConnected() && synapses[i]->GetSource()->IsActive())
            nearActiveSyns++;

    return nearActiveSyns;
}

std::vector<Synapse*> DendriteSegment::GetIsLearningSynapses()
{
    std::vector<Synapse *> learnSyns;

    for (int i=0; i<numSynapses; i++) {
        if (synapses[i]->IsFiring() && synapses[i]->IsLearning())
            learnSyns.push_back(synapses[i]);
    }

    return learnSyns;
}

int DendriteSegment::GetNumIsLearningSynapses()
{
    int learnSyns = 0;

    for (int i=0; i<numSynapses; i++)
        if (synapses[i]->IsFiring() && synapses[i]->IsLearning())
            learnSyns++;
    return learnSyns;
}

