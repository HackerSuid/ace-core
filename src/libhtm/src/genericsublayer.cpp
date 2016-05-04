#include "genericsublayer.h"

GenericSublayer::GenericSublayer()
{
}

GenericSublayer::~GenericSublayer()
{
}

unsigned int GenericSublayer::GetHeight()
{
    return height;
}

unsigned int GenericSublayer::GetWidth()
{
    return width;
}

unsigned int GenericSublayer::GetDepth()
{
    return depth;
}

/*std::vector <std::vector <GenericInput *> > GenericSublayer::GetInput()
{
    return input;
}*/

GenericInput*** GenericSublayer::GetInput()
{
    return input;
}

