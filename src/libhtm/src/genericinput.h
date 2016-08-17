#ifndef GENERICINPUT_H_
#define GENERICINPUT_H_

class Synapse;
class DendriteSegment;

class GenericInput
{
private:
protected:
    int x, y;
    bool active[2], learning[2];
public:
    GenericInput();
    ~GenericInput();
    int GetX() { return x; }
    int GetY() { return y; }
    bool IsActive();
    bool WasActive();
    bool IsLearning() { return learning[0]; }
    bool WasLearning() { return learning[1]; }
    void SetActive(bool flag);
    void SetLearning(bool flag)
    {
        learning[1] = learning[0];
        learning[0] = flag;
    }
};

#endif

