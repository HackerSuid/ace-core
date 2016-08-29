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

class Synapse;
class GenericSublayer;

class DendriteSegment
{
public:
    DendriteSegment(bool sensorimotorFlag);
    ~DendriteSegment();
    bool IsActive();
    bool IsActiveFromLearning();
    void NewSynapse(Synapse *newSyn);
    std::vector<Synapse*> GetIsActiveSynapses();
    std::vector<Synapse*> GetWasActiveSynapses();
    std::vector<Synapse*> GetWasNearActiveSynapses();
    int GetNumIsActiveSynapses();
    int GetNumIsNearActiveSynapses();
    std::vector<Synapse*> GetIsLearningSynapses();
    int GetNumIsLearningSynapses();
    std::vector<Synapse *> GetSynapses();
    int GetNumSynapses();
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

