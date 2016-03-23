#ifndef CODEC_H_
#define CODEC_H_

#include <sys/ptrace.h>

#include "codec_base.h"

#define CALL_OPCODE     0xE8
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

class SensoryCodec : public Codec
{
public:
    SensoryCodec();
    ~SensoryCodec();

    bool Init(char *target_path);

    SensoryRegion* GetPattern();
    bool FirstPattern(SensoryRegion *InputPattern);
    void Reset();

private:
    static bool const registered;
    pid_t child_pid;

    char* ptype_str(size_t pt);
    BITMAPHEADER* ReadBitmapHeader(const char *filename, FILE **fp);
};

#endif

