#ifndef DENDRITE_SEGMENT_H_
#define DENDRITE_SEGMENT_H_

#include <vector>

/* from property of sdrs:
 *
 * - each distributed pattern subsampled using SUBSAMPLE_PERCENT
 *   active distal synapses
 * - SUBSAMPLE_PATTERN_NUM patterns learned on one segment
 * - SUBSAMPLE_PERCENT active synapses indicates only one
 *   of the learned patterns.
 *
 */
#define SUBSAMPLE_PERCENT       0.75
#define SUBSAMPLE_PATTERN_NUM   4
#define SUBSAMPLE_THRESHOLD     SUBSAMPLE_PERCENT

typedef enum {
    SENSORY_DISTAL,
    MOTOR_DISTAL,
    LATERAL_DISTAL,
    LOCATION_DISTAL,
    SENSORY_PROXIMAL
} input_t;

class Synapse;
class GenericSublayer;

class DendriteSegment
{
public:
    DendriteSegment();
    virtual ~DendriteSegment();

    virtual void NewSynapse(Synapse *newSyn);
    virtual bool IsActive();
    virtual bool IsActiveFromLearning();
    virtual bool WasActiveFromLearning();
    virtual unsigned int GetNumIsActiveSynapses();
    virtual std::vector<Synapse*> GetIsActiveSynapses();
    virtual unsigned int GetNumWasActiveSynapses();
    virtual std::vector<Synapse*> GetWasActiveSynapses();
    virtual unsigned int GetNumIsLearningSynapses();
    virtual std::vector<Synapse*> GetWasNearActiveSynapses();
    virtual int GetNumIsNearActiveSynapses();

    /*
    unsigned int GetNumIsActiveSensorySynapses();
    unsigned int GetNumIsActiveMotorSynapses();
    unsigned int GetNumIsActiveLocationSynapses();
    unsigned int GetNumIsActiveLateralSynapses();
    std::vector<Synapse*> GetIsActiveSensorySynapses();
    std::vector<Synapse*> GetIsActiveMotorSynapses();
    std::vector<Synapse*> GetIsActiveLocationSynapses();
    std::vector<Synapse*> GetIsActiveLateralSynapses();

    unsigned int GetNumWasActiveSensorySynapses();
    unsigned int GetNumWasActiveMotorSynapses();
    unsigned int GetNumWasActiveLocationSynapses();
    unsigned int GetNumWasActiveLateralSynapses();
    std::vector<Synapse*> GetWasActiveSensorySynapses();
    std::vector<Synapse*> GetWasActiveMotorSynapses();
    std::vector<Synapse*> GetWasActiveLocationSynapses();
    std::vector<Synapse*> GetWasActiveLateralSynapses();

    unsigned int GetNumIsLearningMotorSynapses();
    unsigned int GetNumIsLearningLocationSynapses();
    unsigned int GetNumIsLearningSensorySynapses();
    unsigned int GetNumIsLearningLateralSynapses();
    std::vector<Synapse*> GetIsLearningSensorySynapses();
    std::vector<Synapse*> GetIsLearningMotorSynapses();
    std::vector<Synapse*> GetIsLearningLocationSynapses();
    std::vector<Synapse*> GetIsLearningLateralSynapses();

    unsigned int GetNumWasLearningSensorySynapses();
    unsigned int GetNumWasLearningMotorSynapses();
    unsigned int GetNumWasLearningLocationSynapses();
    unsigned int GetNumWasLearningLateralSynapses();
    std::vector<Synapse*> GetWasLearningSensorySynapses();
    std::vector<Synapse*> GetWasLearningMotorSynapses();
    std::vector<Synapse*> GetWasLearningLocationSynapses();
    std::vector<Synapse*> GetWasLearningLateralSynapses();
    */

    /*
    bool IsSensorimotor() { return sensorimotorSegment; }
    int GetNumSensorySyns() { return numSensorySyns; }
    int GetNumMotorSyns() { return numMotorSyns; }
    */
    static float GetSubsamplePercent() { return SUBSAMPLE_PERCENT; }
    virtual void RefreshSynapses(GenericSublayer *NewPattern);
    virtual std::vector<Synapse *> GetSynapses()
    {
        return syns;
    }
    /*
    void SetNoTemporalContext() { noTemporalContext = true; }
    bool GetNoTemporalContext() { return noTemporalContext; }
    */
private:
    /*
    bool sensorimotorSegment;
    unsigned int numSensorySyns, numMotorSyns;
    unsigned int numLateralSyns, numLocationSyns;
    std::vector<Synapse *> sensorySyns, motorSyns;
    std::vector<Synapse *> lateralSyns, locationSyns;
    bool noTemporalContext;
    */
protected:
    std::vector<Synapse *> syns;
};

// feedforward synapses on mini-columns
//   - causes action potential
class ProximalDendrite : public DendriteSegment
{
public:
    ProximalDendrite();
    ~ProximalDendrite();
    void NewSynapse(Synapse *newSyn);
    bool IsActive();
    bool IsActiveFromLearning();
    bool WasActiveFromLearning();
    unsigned int GetNumIsActiveSynapses();
    std::vector<Synapse*> GetIsActiveSynapses();
    unsigned int GetNumWasActiveSynapses();
    std::vector<Synapse*> GetWasActiveSynapses();
    unsigned int GetNumIsLearningSynapses();
    std::vector<Synapse*> GetIsLearningSynapses();
    unsigned int GetNumWasLearningSynapses();
    std::vector<Synapse*> GetWasLearningSynapses();
    std::vector<Synapse*> GetWasNearActiveSynapses();
    int GetNumIsNearActiveSynapses();
    void RefreshSynapses(GenericSublayer *NewPattern);
    std::vector<Synapse *> GetSynapses();
private:
};

// prediction context synapses on cells
//   - causes soma depolarization
class DistalDendrite : public DendriteSegment
{
public:
    DistalDendrite();
    ~DistalDendrite();
    void NewSynapse(Synapse *newSyn);
    bool IsActive();
    bool IsActiveFromLearning();
    bool WasActiveFromLearning();
    unsigned int GetNumIsActiveSynapses();
    std::vector<Synapse*> GetIsActiveSynapses();
    unsigned int GetNumWasActiveSynapses();
    std::vector<Synapse*> GetWasActiveSynapses();
    unsigned int GetNumIsLearningSynapses();
    std::vector<Synapse*> GetIsLearningSynapses();
    unsigned int GetNumWasLearningSynapses();
    std::vector<Synapse*> GetWasLearningSynapses();
    std::vector<Synapse*> GetWasNearActiveSynapses();
    int GetNumIsNearActiveSynapses();

    void RefreshSynapses(GenericSublayer *NewPattern);
    std::vector<Synapse *> GetSynapses();
private:
};

#endif

