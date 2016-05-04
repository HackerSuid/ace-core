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
    void SetActive(bool flag);
};

#endif

