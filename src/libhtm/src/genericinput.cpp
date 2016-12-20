#include <stdio.h>
#include <string.h>

#include "genericinput.h"

GenericInput::GenericInput()
{
    memset(&active[0], false, sizeof(bool)*2);
    memset(&learning[0], true, sizeof(bool)*2);
}

GenericInput::~GenericInput()
{
}

