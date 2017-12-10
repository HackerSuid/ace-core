#ifndef HTM_H_
#define HTM_H_

#define XML_LEN             2048

#include "sensoryregion.h"

class Codec;
class HtmRegion;
class HtmSublayer;
class PoolingLayer;

class Htm {
private:
    int logfd;
    char *target_path;
    unsigned int window_w, window_h;
    HtmRegion **regions;
    HtmSublayer **sublayers;
    PoolingLayer *poolingLayer;
    int num_sublayers;
    bool Learning, allowBoosting;
    int seqRstFact;
    Codec *codec;
    unsigned int locCodecSz, locCodecBits;
    SensoryRegion *currentPattern;
public:
    Htm();
    ~Htm();
    void InitHtm(const char *path);
    bool LoadXmlConfig(const char *path);
    void LoadHtmCodec(Codec *codec);
    SensoryRegion* CurrentPattern();
    SensoryRegion* ConsumePattern();
    bool ResetNewObject()
    {
        return currentPattern->GetReset();
    }
    bool FirstPattern();
    void ReloadCodecTarget();
    void PrintPattern(SensoryRegion *pattern);

    int NewSublayer(HtmSublayer *reg);
    void ConnectHeirarchy();
    void ConnectSubcorticalInput(bool refresh);
    void SendInputThroughLayers();

    void GeneratePredCompX11Gnuplot();
    void GeneratePredSpecX11Gnuplot();

    // accessors
    unsigned int GetWindowHeight();
    unsigned int GetWindowWidth();
    unsigned int GetNumSublayers();
    HtmSublayer** GetSublayers();
    char* GetCodecName();
    PoolingLayer* GetPoolingLayer() { return poolingLayer; }
    int GetSeqRstIdx() { return seqRstFact; }
};

#endif

