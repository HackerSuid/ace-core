#ifndef GENERICSUBLAYER_H_
#define GENERICSUBLAYER_H_

class GenericInput;

// base class for an htm sublayer and input region

// this is needed for two reasons:
// 1. an HtmSublayer can't tell if its afferent connection originate from an
//    encoder or another CLA structure, so a generic structure can represent
//    either.
// 2. both columns and encoder units share many function that can be
//    deduplicated in a generic base class, GenericInput, which is contained
//    within this generic base class.
class GenericSublayer
{
protected:
    unsigned int height, width, depth;
    //std::vector <std::vector <GenericInput *> > input;
    GenericInput ***input;
public:
    GenericSublayer();
    ~GenericSublayer();
    unsigned int GetHeight();
    unsigned int GetWidth();
    unsigned int GetDepth();
    //std::vector <std::vector <GenericInput*> > GetInput();
    GenericInput*** GetInput();
};

#endif

