#include <cstddef>
#include <stdlib.h>
#include <stdio.h>

#include "dendritesegment.h"
#include "synapse.h"
#include "genericinput.h"

DendriteSegment::DendriteSegment()
{
    /*
    sensorimotorSegment = sensorimotorFlag;
    numSensorySyns = 0;
    numMotorSyns = 0;
    numLateralSyns = 0;
    numLocationSyns = 0;
    noTemporalContext = false;
    */
}

DendriteSegment::~DendriteSegment()
{
    /*
    std::vector<Synapse *>::iterator it;
    for (it=sensorySyns.begin(); it<sensorySyns.end(); it++)
        delete (*it);
    for (it=motorSyns.begin(); it<motorSyns.end(); it++)
        delete (*it);
    for (it=lateralSyns.begin(); it<lateralSyns.end(); it++)
        delete (*it);
    for (it=locationSyns.begin(); it<locationSyns.end(); it++)
        delete (*it);
    */
}

void DendriteSegment::NewSynapse(Synapse *newSyn)
{
    /*
    if (newSyn->IsSensory())
        sensorySyns.push_back(newSyn);
    else if (newSyn->IsMotor())
        motorSyns.push_back(newSyn);
    else if (newSyn->IsLocation())
        locationSyns.push_back(newSyn);
    else
        lateralSyns.push_back(newSyn);
    */
}

/*
 * If a dendrite segment is attached to a sensorimotor cell, then both
 * sensory pattern and motor patterns are subsampled for synapses. Take this
 * into account when computing its activation.
 */
bool DendriteSegment::IsActive()
{
    /*
    unsigned int activeSen=0, activeMot=0;
    unsigned int activeLat=0, activeLoc=0;
    unsigned int senThreshold=0, motThreshold=0;
    unsigned int latThreshold=0, locThreshold=0;

    if (sensorimotorSegment) {
        activeSen = GetNumIsActiveSensorySynapses();
        //activeMot = GetNumIsActiveMotorSynapses();
        activeLoc = GetNumIsActiveLocationSynapses();
        senThreshold = sensorySyns.size() * SUBSAMPLE_THRESHOLD;
        //motThreshold = motorSyns.size() * SUBSAMPLE_THRESHOLD;
        locThreshold = locationSyns.size() * SUBSAMPLE_THRESHOLD;
        //if (activeSen && activeMot)
            //if (activeSen>=senThreshold&&activeMot>=motThreshold)
        if (activeSen>=senThreshold&&activeLoc>=locThreshold) {
            return true;
        }
    } else {
        activeLat = GetNumIsActiveLateralSynapses();
        latThreshold = lateralSyns.size()*SUBSAMPLE_THRESHOLD;
        if (activeLat && activeLat >= latThreshold)
            return true;
    }

    return false;
    */
}

/*
 * Again, take sensory and motor pattern subsampling into consideration.
 */
bool DendriteSegment::IsActiveFromLearning()
{
    /*
    unsigned int activeSen=0, activeMot=0;
    unsigned int activeLat=0, activeLoc=0;
    unsigned int senThreshold=0, motThreshold=0;
    unsigned int latThreshold=0, locThreshold=0;

    if (sensorimotorSegment) {
        activeSen = GetNumIsLearningSensorySynapses();
        //activeMot = GetNumIsLearningMotorSynapses();
        activeLoc = GetNumIsLearningLocationSynapses();
        senThreshold = sensorySyns.size() * SUBSAMPLE_THRESHOLD;
        //motThreshold = motorSyns.size() * SUBSAMPLE_THRESHOLD;
        locThreshold = locationSyns.size() * SUBSAMPLE_THRESHOLD;
        //if (activeSen && activeMot)
            //if (activeSen>=senThreshold&&activeMot>=motThreshold)
        if (activeSen>=senThreshold&&activeLoc>=locThreshold)
                return true;
    } else {
        activeLat = GetNumIsLearningLateralSynapses();
        latThreshold = lateralSyns.size() * SUBSAMPLE_THRESHOLD;
        if (activeLat && activeLat >= latThreshold)
            return true;
    }

    return false;
    */
}

bool DendriteSegment::WasActiveFromLearning()
{
    /*
    unsigned int activeSen=0, activeMot=0;
    unsigned int activeLat=0, activeLoc=0;
    unsigned int senThreshold=0, motThreshold=0;
    unsigned int latThreshold=0, locThreshold=0;

    if (sensorimotorSegment) {
        activeSen = GetNumWasLearningSensorySynapses();
        //activeMot = GetNumWasLearningMotorSynapses();
        activeLoc = GetNumWasLearningLocationSynapses();
        senThreshold = sensorySyns.size() * SUBSAMPLE_THRESHOLD;
        //motThreshold = motorSyns.size() * SUBSAMPLE_THRESHOLD;
        locThreshold = locationSyns.size() * SUBSAMPLE_THRESHOLD;
        //if (activeSen && activeMot)
            //if (activeSen>=senThreshold&&activeMot>=motThreshold)
        if (activeSen>=senThreshold&&activeLoc>=locThreshold)
                return true;
    } else {
        activeLat = GetNumWasLearningLateralSynapses();
        latThreshold = lateralSyns.size() * SUBSAMPLE_THRESHOLD;
        if (activeLat && activeLat >= latThreshold)
            return true;
    }

    return false;
    */
}

unsigned int DendriteSegment::GetNumIsActiveSynapses()
{
    /*
    return GetIsActiveSynapses().size();
    */
}

std::vector<Synapse*> DendriteSegment::GetIsActiveSynapses()
{
    /*
    std::vector<Synapse*> activeSyns;

    if (sensorimotorSegment) {
        for (unsigned int i=0; i<sensorySyns.size(); i++) {
            if (sensorySyns[i]->IsFiring())
                activeSyns.push_back(sensorySyns[i]);
        }
        //for (unsigned int i=0; i<motorSyns.size(); i++) {
        //    if (motorSyns[i]->IsFiring())
        //        activeSyns.push_back(motorSyns[i]);
        //}
        for (unsigned int i=0; i<locationSyns.size(); i++) {
            if (locationSyns[i]->IsFiring())
                activeSyns.push_back(locationSyns[i]);
        }
    } else {
        for (unsigned int i=0; i<lateralSyns.size(); i++) {
            if (lateralSyns[i]->IsFiring())
                activeSyns.push_back(lateralSyns[i]);
        }
    }

    return activeSyns;
    */
}

unsigned int DendriteSegment::GetNumWasActiveSynapses()
{
    //return GetWasActiveSynapses().size();
}

std::vector<Synapse*> DendriteSegment::GetWasActiveSynapses()
{
    /*
    std::vector<Synapse*> activeSyns;

    if (sensorimotorSegment) {
        for (unsigned int i=0; i<sensorySyns.size(); i++) {
            if (sensorySyns[i]->WasFiring())
                activeSyns.push_back(sensorySyns[i]);
        }
        //for (unsigned int i=0; i<motorSyns.size(); i++) {
        //    if (motorSyns[i]->WasFiring())
        //        activeSyns.push_back(motorSyns[i]);
        //}
        for (unsigned int i=0; i<locationSyns.size(); i++) {
            if (locationSyns[i]->WasFiring()) {
                activeSyns.push_back(locationSyns[i]);
            }
        }
    } else {
        for (unsigned int i=0; i<lateralSyns.size(); i++) {
            if (lateralSyns[i]->WasFiring())
                activeSyns.push_back(lateralSyns[i]);
        }
    }

    return activeSyns;
    */
}

unsigned int DendriteSegment::GetNumIsLearningSynapses()
{
    /*
    //return (sensorimotorSegment ?
    //    GetNumIsLearningSensorySynapses() +
    //    GetNumIsLearningMotorSynapses() :
    //    GetNumIsLearningLateralSynapses()
    //);
    return (sensorimotorSegment ?
        GetNumIsLearningLocationSynapses() +
        GetNumIsLearningSensorySynapses() :
        GetNumIsLearningLateralSynapses()
    );
    */
}

std::vector<Synapse*> DendriteSegment::GetWasNearActiveSynapses()
{
    /*
    std::vector<Synapse*> nearActiveSyns;

    if (sensorimotorSegment) {
        for (unsigned int i=0; i<sensorySyns.size(); i++) {
            if (sensorySyns[i]->IsNearConnected() &&
                sensorySyns[i]->GetSource()->WasActive())
                nearActiveSyns.push_back(sensorySyns[i]);
        }
        //for (unsigned int i=0; i<motorSyns.size(); i++) {
        //    if (motorSyns[i]->IsNearConnected() &&
        //        motorSyns[i]->GetSource()->WasActive())
        //        nearActiveSyns.push_back(motorSyns[i]);
        //}
        for (unsigned int i=0; i<locationSyns.size(); i++) {
            if (locationSyns[i]->IsNearConnected() &&
                locationSyns[i]->GetSource()->WasActive()) {
                nearActiveSyns.push_back(locationSyns[i]);
            }
        }
    } else {
        for (unsigned int i=0; i<lateralSyns.size(); i++) {
            if (lateralSyns[i]->IsNearConnected() &&
                lateralSyns[i]->GetSource()->WasActive())
                nearActiveSyns.push_back(lateralSyns[i]);
        }
    }

    return nearActiveSyns;
    */
}

/*
 * Returns the number of synapses that are nearly connected and active.
 */
int DendriteSegment::GetNumIsNearActiveSynapses()
{
    /*
    int nearActiveSyns = 0;

    if (sensorimotorSegment) {
        for (unsigned int i=0; i<sensorySyns.size(); i++) {
            if (sensorySyns[i]->IsNearConnected() &&
                sensorySyns[i]->GetSource()->IsActive())
                nearActiveSyns++;
        }
        //for (unsigned int i=0; i<motorSyns.size(); i++) {
        //    if (motorSyns[i]->IsNearConnected() &&
        //        motorSyns[i]->GetSource()->IsActive())
        //        nearActiveSyns++;
        //}
        for (unsigned int i=0; i<locationSyns.size(); i++) {
            if (locationSyns[i]->IsNearConnected() &&
                locationSyns[i]->GetSource()->IsActive())
                nearActiveSyns++;
        }
    } else {
        for (unsigned int i=0; i<lateralSyns.size(); i++) {
            if (lateralSyns[i]->IsNearConnected() &&
                lateralSyns[i]->GetSource()->IsActive())
                nearActiveSyns++;
        }
    }

    return nearActiveSyns;
    */
}

/*
unsigned int DendriteSegment::GetNumIsActiveSensorySynapses()
{
    return GetIsActiveSensorySynapses().size();
}

unsigned int DendriteSegment::GetNumIsActiveMotorSynapses()
{
    return GetIsActiveMotorSynapses().size();
}

unsigned int DendriteSegment::GetNumIsActiveLocationSynapses()
{
    return GetIsActiveLocationSynapses().size();
}

unsigned int DendriteSegment::GetNumIsActiveLateralSynapses()
{
    return GetIsActiveLateralSynapses().size();
}

std::vector<Synapse*> DendriteSegment::GetIsActiveSensorySynapses()
{
    std::vector<Synapse *> activeSyns;

    for (unsigned int i=0; i<sensorySyns.size(); i++)
        if (sensorySyns[i]->IsFiring())
            activeSyns.push_back(sensorySyns[i]);

    return activeSyns;
}

std::vector<Synapse*> DendriteSegment::GetIsActiveMotorSynapses()
{   
    std::vector<Synapse *> activeSyns;

    for (unsigned int i=0; i<motorSyns.size(); i++)
        if (motorSyns[i]->IsFiring())
            activeSyns.push_back(motorSyns[i]);

    return activeSyns;
}

std::vector<Synapse*> DendriteSegment::GetIsActiveLocationSynapses()
{
    std::vector<Synapse *> activeSyns;

    for (unsigned int i=0; i<locationSyns.size(); i++)
        if (locationSyns[i]->IsFiring())
            activeSyns.push_back(locationSyns[i]);

    return activeSyns;
}

std::vector<Synapse*> DendriteSegment::GetIsActiveLateralSynapses()
{
    std::vector<Synapse *> activeSyns;

    for (unsigned int i=0; i<lateralSyns.size(); i++)
        if (lateralSyns[i]->IsFiring())
            activeSyns.push_back(lateralSyns[i]);

    return activeSyns;
}

unsigned int DendriteSegment::GetNumWasActiveSensorySynapses()
{
    return GetWasActiveSensorySynapses().size();
}

unsigned int DendriteSegment::GetNumWasActiveMotorSynapses()
{
    return GetWasActiveMotorSynapses().size();
}

unsigned int DendriteSegment::GetNumWasActiveLocationSynapses()
{
    return GetWasActiveLocationSynapses().size();
}

unsigned int DendriteSegment::GetNumWasActiveLateralSynapses()
{
    return GetWasActiveLateralSynapses().size();
}

std::vector<Synapse*> DendriteSegment::GetWasActiveSensorySynapses()
{
    std::vector<Synapse *> activeSyns;

    for (unsigned int i=0; i<sensorySyns.size(); i++)
        if (sensorySyns[i]->WasFiring())
            activeSyns.push_back(sensorySyns[i]);

    return activeSyns;
}

std::vector<Synapse*> DendriteSegment::GetWasActiveMotorSynapses()
{   
    std::vector<Synapse *> activeSyns;

    for (unsigned int i=0; i<motorSyns.size(); i++)
        if (motorSyns[i]->WasFiring())
            activeSyns.push_back(motorSyns[i]);

    return activeSyns;
}

std::vector<Synapse*> DendriteSegment::GetWasActiveLocationSynapses()
{
    std::vector<Synapse *> activeSyns;

    for (unsigned int i=0; i<locationSyns.size(); i++)
        if (locationSyns[i]->WasFiring())
            activeSyns.push_back(locationSyns[i]);

    return activeSyns;
}

std::vector<Synapse*> DendriteSegment::GetWasActiveLateralSynapses()
{
    std::vector<Synapse *> activeSyns;

    for (unsigned int i=0; i<lateralSyns.size(); i++)
        if (lateralSyns[i]->WasFiring())
            activeSyns.push_back(lateralSyns[i]);

    return activeSyns;
}

unsigned int DendriteSegment::GetNumIsLearningSensorySynapses()
{
    return GetIsLearningSensorySynapses().size();
}

unsigned int DendriteSegment::GetNumIsLearningMotorSynapses()
{
    return GetIsLearningMotorSynapses().size();
}

unsigned int DendriteSegment::GetNumIsLearningLateralSynapses()
{
    return GetIsLearningLateralSynapses().size();
}

unsigned int DendriteSegment::GetNumIsLearningLocationSynapses()
{
    return GetIsLearningLocationSynapses().size();
}

std::vector<Synapse*> DendriteSegment::GetIsLearningSensorySynapses()
{
    std::vector<Synapse *> learnSyns;

    for (unsigned int i=0; i<sensorySyns.size(); i++) {
        if (sensorySyns[i]->IsFiring() && sensorySyns[i]->IsLearning())
            learnSyns.push_back(sensorySyns[i]);
    }

    return learnSyns;
}

std::vector<Synapse*> DendriteSegment::GetIsLearningMotorSynapses()
{
    std::vector<Synapse *> learnSyns;

    for (unsigned int i=0; i<motorSyns.size(); i++) {
        if (motorSyns[i]->IsFiring() && motorSyns[i]->IsLearning())
            learnSyns.push_back(motorSyns[i]);
    }

    return learnSyns;
}

std::vector<Synapse*> DendriteSegment::GetIsLearningLocationSynapses()
{
    std::vector<Synapse *> learnSyns;

    for (unsigned int i=0; i<locationSyns.size(); i++) {
        if (locationSyns[i]->IsFiring() && locationSyns[i]->IsLearning())
            learnSyns.push_back(locationSyns[i]);
    }

    return learnSyns;
}

std::vector<Synapse*> DendriteSegment::GetIsLearningLateralSynapses()
{
    std::vector<Synapse *> learnSyns;

    for (unsigned int i=0; i<lateralSyns.size(); i++) {
        if (lateralSyns[i]->IsFiring() && lateralSyns[i]->IsLearning())
            learnSyns.push_back(lateralSyns[i]);
    }

    return learnSyns;
}

unsigned int DendriteSegment::GetNumWasLearningSensorySynapses()
{
    return GetWasLearningSensorySynapses().size();
}

unsigned int DendriteSegment::GetNumWasLearningMotorSynapses()
{
    return GetWasLearningMotorSynapses().size();
}

unsigned int DendriteSegment::GetNumWasLearningLocationSynapses()
{
    return GetWasLearningLocationSynapses().size();
}

unsigned int DendriteSegment::GetNumWasLearningLateralSynapses()
{
    return GetWasLearningLateralSynapses().size();
}

std::vector<Synapse*> DendriteSegment::GetWasLearningSensorySynapses()
{
    std::vector<Synapse *> learnSyns;

    for (unsigned int i=0; i<sensorySyns.size(); i++) {
        if (sensorySyns[i]->WasFiring() && sensorySyns[i]->WasLearning())
            learnSyns.push_back(sensorySyns[i]);
    }

    return learnSyns;
}

std::vector<Synapse*> DendriteSegment::GetWasLearningMotorSynapses()
{
    std::vector<Synapse *> learnSyns;

    for (unsigned int i=0; i<motorSyns.size(); i++) {
        if (motorSyns[i]->WasFiring() && motorSyns[i]->WasLearning())
            learnSyns.push_back(motorSyns[i]);
    }

    return learnSyns;
}

std::vector<Synapse*> DendriteSegment::GetWasLearningLocationSynapses()
{
    std::vector<Synapse *> learnSyns;

    for (unsigned int i=0; i<locationSyns.size(); i++) {
        if (locationSyns[i]->WasFiring() && locationSyns[i]->WasLearning())
            learnSyns.push_back(locationSyns[i]);
    }

    return learnSyns;
}

std::vector<Synapse*> DendriteSegment::GetWasLearningLateralSynapses()
{
    std::vector<Synapse *> learnSyns;

    for (unsigned int i=0; i<lateralSyns.size(); i++) {
        if (lateralSyns[i]->WasFiring() && lateralSyns[i]->WasLearning())
            learnSyns.push_back(lateralSyns[i]);
    }

    return learnSyns;
}
*/

void DendriteSegment::RefreshSynapses(GenericSublayer *NewPattern)
{
    /*
    if (sensorimotorSegment) {
        for (unsigned int i=0; i<sensorySyns.size(); i++)
            sensorySyns[i]->RefreshSynapse(NewPattern);
        //for (unsigned int i=0; i<motorSyns.size(); i++)
        //    motorSyns[i]->RefreshSynapse(NewPattern);
        for (unsigned int i=0; i<locationSyns.size(); i++) {
            //printf("refreshing synapse %u\n", i);
            locationSyns[i]->RefreshSynapse(NewPattern);
        }
    } else {
        for (unsigned int i=0; i<lateralSyns.size(); i++)
            lateralSyns[i]->RefreshSynapse(NewPattern);
    }
    */
}

/*
std::vector<Synapse *> DendriteSegment::GetSynapses()
{
    std::vector<Synapse *> synapses;

    for (unsigned int i=0; i<sensorySyns.size(); i++)
        synapses.push_back(sensorySyns[i]);
    for (unsigned int i=0; i<motorSyns.size(); i++)
        synapses.push_back(motorSyns[i]);
    for (unsigned int i=0; i<locationSyns.size(); i++)
        synapses.push_back(locationSyns[i]);
    for (unsigned int i=0; i<lateralSyns.size(); i++)
        synapses.push_back(lateralSyns[i]);

    return synapses;
}
*/

/* Proximal subclass */

ProximalDendrite::ProximalDendrite()
    : DendriteSegment()
{
}

ProximalDendrite::~ProximalDendrite()
{
    std::vector<Synapse *>::iterator it;
    for (it=syns.begin(); it<syns.end(); it++)
        delete (*it);
}

// TODO: refactor into parent class
void ProximalDendrite::NewSynapse(Synapse *newSyn)
{
    syns.push_back(newSyn);
}

bool ProximalDendrite::IsActive()
{
    float n = syns.size() * SUBSAMPLE_THRESHOLD;

    return (GetNumIsActiveSynapses() >= n);
}

bool ProximalDendrite::IsActiveFromLearning()
{
    float n = syns.size() * SUBSAMPLE_THRESHOLD;

    return (GetNumIsLearningSynapses() >= n);
}

bool ProximalDendrite::WasActiveFromLearning()
{
    float n = syns.size() * SUBSAMPLE_THRESHOLD;

    return (GetNumWasLearningSynapses() >= n);
}

unsigned int ProximalDendrite::GetNumIsActiveSynapses()
{
    return GetIsActiveSynapses().size();
}

std::vector<Synapse*> ProximalDendrite::GetIsActiveSynapses()
{
    std::vector <Synapse *> activeSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->IsFiring())
            activeSyns.push_back(syns[i]);

    return activeSyns;
}

unsigned int ProximalDendrite::GetNumWasActiveSynapses()
{
    return GetWasActiveSynapses().size();
}

std::vector<Synapse*> ProximalDendrite::GetWasActiveSynapses()
{
    std::vector <Synapse *> activeSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->WasFiring())
            activeSyns.push_back(syns[i]);

    return activeSyns;
}

unsigned int ProximalDendrite::GetNumIsLearningSynapses()
{
    return GetIsLearningSynapses().size();
}

std::vector<Synapse*> ProximalDendrite::GetIsLearningSynapses()
{
    std::vector <Synapse *> learnSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->IsFiring() && syns[i]->IsLearning())
            learnSyns.push_back(syns[i]);

    return learnSyns;
}

unsigned int ProximalDendrite::GetNumWasLearningSynapses()
{
    return GetWasLearningSynapses().size();
}

std::vector<Synapse*> ProximalDendrite::GetWasLearningSynapses()
{
    std::vector <Synapse *> learnSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->WasFiring() && syns[i]->WasLearning())
            learnSyns.push_back(syns[i]);

    return learnSyns;
}

std::vector<Synapse*> ProximalDendrite::GetWasNearActiveSynapses()
{
    std::vector<Synapse*> nearActiveSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->IsNearConnected() && syns[i]->GetSource()->WasActive())
            nearActiveSyns.push_back(syns[i]);

    return nearActiveSyns;
}

int ProximalDendrite::GetNumIsNearActiveSynapses()
{
    int nearActiveSyns=0;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->IsNearConnected() && syns[i]->GetSource()->IsActive())
            nearActiveSyns++;

    return nearActiveSyns;
}

void ProximalDendrite::RefreshSynapses(GenericSublayer *NewPattern)
{
    for (unsigned int i=0; i<syns.size(); i++)
        syns[i]->RefreshSynapse(NewPattern);
}

std::vector<Synapse *> ProximalDendrite::GetSynapses()
{
    return syns;
}

/* Distal subclass */

DistalDendrite::DistalDendrite()
    : DendriteSegment()
{
}

DistalDendrite::~DistalDendrite()
{
    std::vector<Synapse *>::iterator it;
    for (it=syns.begin(); it<syns.end(); it++)
        delete (*it);
}

// TODO refactor into parent class
void DistalDendrite::NewSynapse(Synapse *newSyn)
{
    syns.push_back(newSyn);
}

bool DistalDendrite::IsActive()
{
    float n = syns.size() * SUBSAMPLE_THRESHOLD;

    return (GetNumIsActiveSynapses() >= n);
}

bool DistalDendrite::IsActiveFromLearning()
{
    float n = syns.size() * SUBSAMPLE_THRESHOLD;

    return (GetNumIsLearningSynapses() >= n);
}

bool DistalDendrite::WasActiveFromLearning()
{
    float n = syns.size() * SUBSAMPLE_THRESHOLD;

    return (GetNumWasLearningSynapses() >= n);
}

unsigned int DistalDendrite::GetNumIsActiveSynapses()
{
    return GetIsActiveSynapses().size();
}

std::vector<Synapse*> DistalDendrite::GetIsActiveSynapses()
{
    std::vector <Synapse *> activeSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->IsFiring())
            activeSyns.push_back(syns[i]);

    return activeSyns;
}

unsigned int DistalDendrite::GetNumWasActiveSynapses()
{
    return GetWasActiveSynapses().size();
}

std::vector<Synapse*> DistalDendrite::GetWasActiveSynapses()
{
    std::vector <Synapse *> activeSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->WasFiring())
            activeSyns.push_back(syns[i]);

    return activeSyns;
}

unsigned int DistalDendrite::GetNumIsLearningSynapses()
{
    return GetIsLearningSynapses().size();
}

std::vector<Synapse*> DistalDendrite::GetIsLearningSynapses()
{
    std::vector <Synapse *> learnSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->IsFiring() && syns[i]->IsLearning())
            learnSyns.push_back(syns[i]);

    return learnSyns;
}

unsigned int DistalDendrite::GetNumWasLearningSynapses()
{
    return GetWasLearningSynapses().size();
}

std::vector<Synapse*> DistalDendrite::GetWasLearningSynapses()
{
    std::vector <Synapse *> learnSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->WasFiring() && syns[i]->WasLearning())
            learnSyns.push_back(syns[i]);

    return learnSyns;
}

std::vector<Synapse*> DistalDendrite::GetWasNearActiveSynapses()
{
    std::vector<Synapse*> nearActiveSyns;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->IsNearConnected() && syns[i]->GetSource()->WasActive())
            nearActiveSyns.push_back(syns[i]);

    return nearActiveSyns;
}

int DistalDendrite::GetNumIsNearActiveSynapses()
{
    int nearActiveSyns=0;

    for (unsigned int i=0; i<syns.size(); i++)
        if (syns[i]->IsNearConnected() && syns[i]->GetSource()->IsActive())
            nearActiveSyns++;

    return nearActiveSyns;
}

void DistalDendrite::RefreshSynapses(GenericSublayer *NewPattern)
{
    for (unsigned int i=0; i<syns.size(); i++)
        syns[i]->RefreshSynapse(NewPattern);
}

std::vector<Synapse *> DistalDendrite::GetSynapses()
{
    return syns;
}

