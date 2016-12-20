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
    SENSORY_PROXIMAL
} input_t;

class Synapse;
class GenericSublayer;

class DendriteSegment
{
public:
    DendriteSegment(bool sensorimotorFlag);
    ~DendriteSegment();
    bool IsActive();
    bool IsActiveFromLearning();
    bool WasActiveFromLearning();
    bool IsSensorimotor() { return sensorimotorSegment; }
    void NewSynapse(Synapse *newSyn);
    std::vector<Synapse *> GetSynapses();
    int GetNumSynapses();
    std::vector<Synapse*> GetIsActiveSynapses();
    unsigned int GetNumIsActiveSynapses();
    std::vector<Synapse*> GetWasActiveSynapses();
    std::vector<Synapse*> GetWasNearActiveSynapses();
    int GetNumIsNearActiveSynapses();
    std::vector<Synapse*> GetIsLearningSynapses();
    std::vector<Synapse*> GetWasLearningSynapses();
    unsigned int GetNumIsLearningSynapses();
    unsigned int GetNumWasLearningSynapses();
    int GetNumSensorySyns() { return numSensorySyns; }
    int GetNumMotorSyns() { return numMotorSyns; }
    static float GetSubsamplePercent() { return SUBSAMPLE_PERCENT; }
    void RefreshSynapses(GenericSublayer *NewPattern);
    void SetNoTemporalContext();
    bool GetNoTemporalContext();
private:
    bool sensorimotorSegment;
    int numSensorySyns, numMotorSyns;
    std::vector<Synapse *> synapses;
    bool noTemporalContext;
};

#endif

