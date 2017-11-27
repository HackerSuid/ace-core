#ifndef CODEC_BASE_H_
#define CODEC_BASE_H_

#include <stdio.h>

class SensoryRegion;
class HtmSublayer;

// an abstract base class for different types of polymorphic Codec classes
class Codec
{
protected:
    char *codecName;
    char *targetPath;
    int pidx;
public:
    Codec();
    ~Codec();

    template <typename D>
    static bool const Register()
    {
        Instance = new D();
        return true;
    }
    static Codec* Instantiate()
    {
        return Instance;
    }

    virtual bool Init(
        char *target_path,
        HtmSublayer *sensoryLayer,
        unsigned int locSize,
        unsigned int locActiveBits) = 0;
    virtual SensoryRegion* GetPattern(bool Learning) = 0;
    virtual bool FirstPattern() { return true; }
    virtual bool Reset() { return true; }
    char* GetCodecName();
private:
    static Codec *Instance;
};

#endif

