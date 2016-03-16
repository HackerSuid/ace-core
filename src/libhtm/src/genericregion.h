#ifndef GENERIC_H_
#define GENERIC_H_

class GenericInput;

// base class for an htm region and input region - "It's all cells!"

// this is needed for two reasons:
// 1. an HtmRegion can't tell if its afferent connection originate from an
//    encoder or another CLA structure, so a generic structure can represent
//    either.
// 2. both columns and encoder units share many function that can be
//    deduplicated in a generic base class, GenericInput, which is contained
//    within this generic base class.
class GenericRegion
{
protected:
    unsigned int height, width, depth;
    //std::vector <std::vector <GenericInput *> > input;
    GenericInput ***input;
public:
    GenericRegion();
    ~GenericRegion();
    unsigned int GetHeight();
    unsigned int GetWidth();
    unsigned int GetDepth();
    //std::vector <std::vector <GenericInput*> > GetInput();
    GenericInput*** GetInput();
};

#endif

