#ifndef HTM_H_
#define HTM_H_

#define XML_LEN             2048

class Codec;
class SensoryRegion;
class HtmRegion;
class HtmSublayer;

class Htm {
private:
    char *target_path;
    unsigned int window_w, window_h;
    HtmRegion **regions;
    HtmSublayer **sublayers;
    int num_sublayers;
    bool Learning, allowBoosting;
    int seqRstFact;
    Codec *codec;
    SensoryRegion *currentPattern;
public:
    Htm();
    ~Htm();
    void InitHtm(const char *path);
    bool LoadXmlConfig(const char *path);
    void LoadHtmCodec(Codec *codec);
    SensoryRegion* CurrentPattern();
    SensoryRegion* ConsumePattern();
    bool FirstPattern();
    void ResetCodec();
    void PrintPattern(SensoryRegion *pattern);

    int NewSublayer(HtmSublayer *reg);
    void ConnectHeirarchy();
    void ConnectSubcorticalInput(bool refresh);
    void PushNextClaInput();

    void GeneratePredCompX11Gnuplot();
    void GeneratePredSpecX11Gnuplot();

    // accessors
    unsigned int GetWindowHeight();
    unsigned int GetWindowWidth();
    unsigned int GetNumSublayers();
    HtmSublayer** GetSublayers();
    char* GetCodecName();
    int GetSeqRstIdx() { return seqRstFact; }
};

#endif

