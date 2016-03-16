#ifndef GENERICINPUT_H_
#define GENERICINPUT_H_

class Synapse;
class DendriteSegment;

class GenericInput
{
private:
protected:
    bool active[2], learning[2];
public:
    GenericInput();
    ~GenericInput();
    bool IsActive();
    bool WasActive();
    void SetActive(bool flag);
};

#endif

