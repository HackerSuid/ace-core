#ifndef HTM_H_
#define HTM_H_

#define XML_LEN             2048
#define LOCAL_XML_CONF_PATH "ace.conf"

class Codec;
class SensoryRegion;
class HtmRegion;

class Htm {
private:
    char *target_path;
    unsigned int window_w, window_h;
    HtmRegion **regions;
    int num_regions;
    bool Learning, allowBoosting;
    int seqRstFact;
    Codec *codec;
    SensoryRegion *currentPattern;
public:
    Htm();
    ~Htm();
    void InitHtm();
    bool LoadXmlConfig(const char *path);
    void LoadHtmCodec(Codec *codec);
    SensoryRegion* CurrentPattern();
    SensoryRegion* ConsumePattern();
    bool FirstPattern();
    void ResetCodec();
    void PrintPattern(SensoryRegion *pattern);

    int new_region(HtmRegion *reg);
    void ConnectHeirarchy();
    void ConnectSensoryRegion(bool refresh);
    void CLA();
    // accessors
    unsigned int GetWindowHeight();
    unsigned int GetWindowWidth();
    unsigned int GetNumRegions();
    HtmRegion** GetRegions();
    char* GetCodecName();
    int GetSeqRstIdx() { return seqRstFact; }
};

#endif

