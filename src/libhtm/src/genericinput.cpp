#include <stdio.h>
#include <string.h>

#include "genericinput.h"

GenericInput::GenericInput()
{
    memset(&active[0], false, sizeof(bool)*2);
}

GenericInput::~GenericInput()
{
}

bool GenericInput::IsActive()
{
    return active[0];
}

bool GenericInput::WasActive()
{
    return active[1];
}

void GenericInput::SetActive(bool flag)
{
    active[1] = active[0];
    active[0] = flag;
}

