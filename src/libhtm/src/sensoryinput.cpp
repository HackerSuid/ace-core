#include "sensoryinput.h"

SensoryInput::SensoryInput(int x, int y)
{
    this->x = x;
    this->y = y;
    // sensory inputs always considered for learning.
    memset(&learning[0], true, sizeof(bool)*2);
    // set WasActive to true, and IsActive to false
    active[1] = true;
    active[0] = false;
}

SensoryInput::~SensoryInput()
{
}

