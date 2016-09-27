#ifndef ELFCODEC_H_
#define ELFCODEC_H_

#include <sys/ptrace.h>
#include <vector>
#include <map>

#include "codec_base.h"

#define CALL_OPCODE     0xE8
#define RET_OPCODE      0xC3
#define SOFT_INT_OPCODE 0x80CD

#pragma pack(1)     // pack structures to avoid memory padding
typedef struct
{
    unsigned short magic;       // bitmap signature - "BM"
    unsigned int bfsize;        // bitmap file size
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;        // offset address to pixel array
} BITMAPFILEHEADER;

typedef struct
{
    unsigned int size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bitsperpixel;
    unsigned int compression;
    unsigned int imagesz;
    int xresolution;
    int yresolution;
    unsigned int colorsused;
    unsigned int importantcolors;
} BITMAPINFOHEADER;
#pragma pack(0)

typedef struct
{
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
} BITMAPHEADER;

// assuming 24-bit bitmap
typedef struct
{
    // 3 byte RGB
    unsigned char r, g, b;
} Pixel;

// custom std::map comparator for char pointers
struct strcmpr
{
    bool operator()(const char *a, const char *b) const
    {
        return strcmp(a, b);
    }
};

class SensoryCodec;
class SensoryInput;

typedef struct
{
    int fd;
    SensoryCodec *codec;
} SensoryCodecBinding;

class SensoryCodec
{
public:
    virtual ~SensoryCodec() {}
    virtual SensoryRegion* GetPattern(
        int sense_fd,
        bool Learning
    ) = 0;
};

class BitmapCodec : public SensoryCodec
{
public:
    BitmapCodec();
    ~BitmapCodec();

    SensoryRegion* GetPattern(int sense_fd, bool Learning);

    BITMAPHEADER* ReadBitmapHeader(int);
};

class SensoryCodecFactory
{
public:
    ~SensoryCodecFactory()
    {
        typename std::map<const char *, SensoryCodec *, strcmpr>::iterator it =
            CodecCtorMap.begin();
        while (it != CodecCtorMap.end())
        {
            delete (*it).second;
            ++it;
        }
    }

    void Register(const char *id, SensoryCodec *Ctor)
    {
        CodecCtorMap[id] = Ctor;
    }

    SensoryCodec* Get(const char *id)
    {
        return CodecCtorMap[id];
    }

private:
    std::map<const char *, SensoryCodec *, strcmpr> CodecCtorMap;
};

class Autoencoder;

class ElfCodec : public Codec
{
public:
    ElfCodec();
    ~ElfCodec();

    bool Init(
        char *target_path,
        unsigned int height,
        unsigned int width,
        float localActivity
    );
    bool LoadTarget();
    unsigned int ExecuteToCall(
        std::vector<unsigned int>,
        struct user_regs_struct*
    );
    SensoryRegion* GetPattern(bool Learning);
    SensoryCodecBinding HandlePureSensory(struct user_regs_struct *regs);
    void AddNewMotorEncoding(unsigned int motorCallAddr);
    int GetRewardSignal();
    bool FirstPattern();
    bool Reset();

private:
    // private functions
    char* ptype_str(size_t pt);

    // private data
    SensoryCodecFactory sensoryCodecFactory;

    static bool const registered;
    bool firstPattern;
    pid_t child_pid;
    char *child_dir;
    int wait_status;
    unsigned codecHeight, codecWidth;
    float codecActiveRatio;

    // deprecated encoder technique
    unsigned int read_plt, open_plt, main_addr;

    std::map<unsigned int, SensoryRegion *> motorCommandEncodings;
    SensoryInput ***masterMotorEncoding;
    std::vector<unsigned int> pureSensoryFunctions;
    std::vector<unsigned int> cpgFunctions;
    std::vector<unsigned int> sensorimotorFunctions;

    // latest encoder technique
    std::vector< std::vector<unsigned int> > localFuncAddrs;
    std::vector<unsigned int> dynFuncPltAddrs;
    std::vector< std::vector<unsigned char> > fcnMachCode;

    Autoencoder *ae;
};

#endif

