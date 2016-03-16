#include "genericregion.h"

GenericRegion::GenericRegion()
{
}

GenericRegion::~GenericRegion()
{
}

unsigned int GenericRegion::GetHeight()
{
    return height;
}

unsigned int GenericRegion::GetWidth()
{
    return width;
}

unsigned int GenericRegion::GetDepth()
{
    return depth;
}

/*std::vector <std::vector <GenericInput *> > GenericRegion::GetInput()
{
    return input;
}*/

GenericInput*** GenericRegion::GetInput()
{
    return input;
}

