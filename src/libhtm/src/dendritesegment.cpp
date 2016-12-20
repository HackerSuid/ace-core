#include <cstddef>
#include <stdlib.h>
#include <stdio.h>

#include "dendritesegment.h"
#include "synapse.h"
#include "genericinput.h"

DendriteSegment::DendriteSegment(bool sensorimotorFlag)
{
    sensorimotorSegment = sensorimotorFlag;
    numSensorySyns = 0;
    numMotorSyns = 0;
    // flag to indicate no temporal context as the first sequence
    // pattern
    noTemporalContext = false;
}

DendriteSegment::~DendriteSegment()
{
    std::vector<Synapse *>::iterator it;
    for (it=synapses.begin(); it<synapses.end(); it++)
        delete (*it);
}

/*
 * If a dendrite segment is attached to a sensorimotor cell, then both
 * sensory pattern and motor patterns are subsampled for synapses. Take this
 * into account when computing its activation.
 */
bool DendriteSegment::IsActive()
{
    int activeSyns = GetNumIsActiveSynapses();
    int threshold = synapses.size() * SUBSAMPLE_THRESHOLD;
    if (activeSyns && (activeSyns >= threshold))
        return true;
    return false;
}

/*
 * Again, take sensory and motor pattern subsampling into consideration.
 */
bool DendriteSegment::IsActiveFromLearning()
{
    int activeLearnSyns = GetNumIsLearningSynapses();
    int threshold = synapses.size() * SUBSAMPLE_THRESHOLD;
    printf("\t\tof %u syns, need %d & have %d\n",
        synapses.size(), threshold, activeLearnSyns);
    if (activeLearnSyns && (activeLearnSyns >= threshold))
        return true;
    return false;
}

void DendriteSegment::NewSynapse(Synapse *newSyn)
{
    synapses.push_back(newSyn);

    if (newSyn->IsMotor())
        numMotorSyns++;
    else
        numSensorySyns++;
}

void DendriteSegment::RefreshSynapses(GenericSublayer *NewPattern)
{
    for (unsigned int i=0; i<synapses.size(); i++)
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

    for (unsigned int i=0; i<synapses.size(); i++)
        if (synapses[i]->IsFiring())
            activeSyns.push_back(synapses[i]);

    return activeSyns;
}

unsigned int DendriteSegment::GetNumIsActiveSynapses()
{
    return GetIsActiveSynapses().size();
}

/* Same as above but for the previous timestep */
std::vector<Synapse*> DendriteSegment::GetWasActiveSynapses()
{
    std::vector<Synapse*> activeSyns;

    printf("\tchecking WasFiring() on %u syns\n", synapses.size());
    for (unsigned int i=0; i<synapses.size(); i++) {
        printf("\t\tsyn %u %d:%d:%d [0x%08x]\n",
            i, synapses[i]->IsLateral(),
            synapses[i]->IsSensory(),
            synapses[i]->IsMotor(),
            synapses[i]);
        if (synapses[i]->WasFiring())
            activeSyns.push_back(synapses[i]);
    }

    return activeSyns;
}

std::vector<Synapse*> DendriteSegment::GetWasNearActiveSynapses()
{
    std::vector<Synapse*> nearActiveSyns;

    for (unsigned int i=0; i<synapses.size(); i++)
        if (synapses[i]->IsNearConnected() && synapses[i]->GetSource()->WasActive())
            nearActiveSyns.push_back(synapses[i]);

    return nearActiveSyns;
}

/*
 * Returns the number of synapses that are nearly connected and active.
 */
int DendriteSegment::GetNumIsNearActiveSynapses()
{
    int nearActiveSyns = 0;

    for (unsigned int i=0; i<synapses.size(); i++)
        if (synapses[i]->IsNearConnected() && synapses[i]->GetSource()->IsActive())
            nearActiveSyns++;

    return nearActiveSyns;
}

std::vector<Synapse*> DendriteSegment::GetIsLearningSynapses()
{
    std::vector<Synapse *> learnSyns;

    for (unsigned int i=0; i<synapses.size(); i++) {
        if (synapses[i]->IsFiring() && synapses[i]->IsLearning())
            learnSyns.push_back(synapses[i]);
    }

    return learnSyns;
}

unsigned int DendriteSegment::GetNumIsLearningSynapses()
{
    return GetIsLearningSynapses().size();
}

void DendriteSegment::SetNoTemporalContext()
{
    noTemporalContext = true;
}

bool DendriteSegment::GetNoTemporalContext()
{
    return noTemporalContext;
}

