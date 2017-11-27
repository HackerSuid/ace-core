#include <vector>
#include <algorithm>

class SensoryRegion;

// need to refactor Codec base API to work with this
class LocationCodec
{
public:
    LocationCodec(unsigned int n, unsigned int w, unsigned int d);
    ~LocationCodec();
    void initBucketMap();
    std::vector<unsigned int> randomRepresentation(
        unsigned int n,
        unsigned int w
    );
    std::vector<unsigned int> listRange(unsigned int n);
    bool Init();
    SensoryRegion* GetPattern(int inode);
    unsigned int getBucketIdx(int inode);
    void createNewBucket(unsigned int idx);
    std::vector<unsigned int> newRepresentation(unsigned int idx);
    char* GetCodecName() { return (char *)"LocationCodec"; }
private:
    unsigned int n, w, d;
    unsigned minIndex, maxIndex;
    unsigned maxBuckets;
    unsigned int offset, resolution;
    std::map<unsigned int, std::vector<unsigned int>> bucketMap;
    SensoryRegion *locPattern;
    SensoryInput ***locInputs;
};
