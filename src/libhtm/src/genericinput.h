#ifndef GENERICINPUT_H_
#define GENERICINPUT_H_

class Synapse;
class DendriteSegment;

class GenericInput
{
private:
protected:
    int x, y;
    bool active[2], learning[2], predicted[2];
public:
    GenericInput();
    ~GenericInput();
    int GetX() { return x; }
    int GetY() { return y; }
    bool IsActive() { return active[0]; }
    bool WasActive() { return active[1]; }
    bool IsLearning() { return learning[0]; }
    bool WasLearning() { return learning[1]; }
    bool IsPredicted() { return predicted[0]; }
    bool WasPredicted() { return predicted[1]; }
    void SetActive(bool flag)
    {
        active[1] = active[0];
        active[0] = flag;
    }
    void SetLearning(bool flag)
    {
        learning[1] = learning[0];
        learning[0] = flag;
    }
    void SetPredicted(bool flag)
    {
        predicted[1] = predicted[0];
        predicted[0] = flag;
    }
};

#endif

