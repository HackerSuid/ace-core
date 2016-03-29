#ifndef CODEC_BASE_H_
#define CODEC_BASE_H_

#include <stdio.h>

class SensoryRegion;

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

    virtual bool Init(char *target_path) = 0;
    virtual SensoryRegion* GetPattern() = 0;
    virtual bool FirstPattern() = 0;
    virtual bool Reset() = 0;
    char* GetCodecName();
private:
    static Codec *Instance;
};

/* Remove this when I no longer need as reference.
class ElfApiCodec : public Codec
{
public:
    ElfApiCodec();
    ~ElfApiCodec();
    static ElfApiCodec* ElfApiCodecFactory();
    SensoryRegion* CurrentPattern();
    SensoryRegion* ConsumePattern();
    bool FirstPattern();
};
*/

#endif

