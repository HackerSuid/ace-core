#ifndef SENSORYINPUT_H_
#define SENSORYINPUT_H_

#include "genericinput.h"
#include <string.h>

class SensoryInput : public GenericInput
{
private:
public:
    SensoryInput(int x, int y);
    ~SensoryInput();
};

#endif

