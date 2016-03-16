#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <rapidxml/rapidxml.hpp>

#include "htm.h"
#include "htmregion.h"
#include "sensoryregion.h"
#include "sensoryinput.h"
#include "codec.h"
#include "util.h"

Htm::Htm()
{
    regions = NULL;
    num_regions = 0;
    Learning = true;
    currentPattern = NULL;
}

Htm::~Htm()
{
}

void Htm::InitHtm()
{
    // initialization of Htm regions and columns.
    if (!LoadXmlConfig(LOCAL_XML_CONF_PATH))
        abort();
    // instantiation of the codec.
    if (!(codec = Codec::Instantiate()))
        abort();
    // initialization of the codec.
    if (!codec->Init(target_path))
        abort();
    // initialization and linkage of the regions.
    this->ConnectHeirarchy();
}

bool Htm::LoadXmlConfig(const char *pathname)
{
    int xmlfd;
    char conf_xml[XML_LEN];

    // open and read the xml configuration file
    if ((xmlfd = open(pathname, O_RDONLY)) < 0) {
        fprintf(stderr, "Failed to open %s for reading.\n", pathname);
        return false;
    }
    if (read(xmlfd, conf_xml, XML_LEN) < 0) {
        fprintf(stderr, "Failed to read %s.\n", pathname);
        return false;
    }

    // the xml dom object and load the xml file
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_no_data_nodes>((char *)conf_xml);
    // Load the HTM and pass parameters to the constructor.
    rapidxml::xml_node<> *HtmNode = doc.first_node("Htm");
    if (!(target_path = HtmNode->first_attribute("target")->value())) {
        fprintf(stderr, "Librarian target missing from configuration file.\n");
        return false;
    }
    window_w = atoi(HtmNode->first_attribute("WinWidth")->value());
    window_h = atoi(HtmNode->first_attribute("WinHeight")->value());
    allowBoosting = atob(HtmNode->first_attribute("allowBoosting")->value());
    // Add regions to the Htm
    rapidxml::xml_node<> *reg = HtmNode->first_node("Region");
    if (!reg) {
        fprintf(stderr, "No regions found in configuration!\n");
        return false;
    }
    do {
        unsigned int h = atoi(reg->first_attribute("height")->value());
        unsigned int w = atoi(reg->first_attribute("width")->value());
        unsigned int cpc = atoi(reg->first_attribute("cellsPerCol")->value());
        //printf("%d columns: %d x %d.\n", h*w, h, w);
        HtmRegion *curr = new HtmRegion(h, w, cpc, this);

        rapidxml::xml_node<> *cols = reg->first_node("Columns");
        float rfsz = atof(cols->first_attribute("recFieldSz")->value());
        float localActivity = atof(cols->first_attribute("localActivity")->value());
        float columnComplexity = atof(cols->first_attribute("columnComplexity")->value());
        bool highTier = atob(cols->first_attribute("highTier")->value());
        int activityCycleWindow = atoi(cols->first_attribute("activityCycleWindow")->value());
        curr->AllocateColumns(
            rfsz,
            localActivity,
            columnComplexity,
            highTier,
            activityCycleWindow
        );

        this->new_region(curr);
    } while ((reg = reg->next_sibling("Region")));

    close(xmlfd);
    return true;
}

// Set lower/higher pointers and initialize potential synapses.
void Htm::ConnectHeirarchy()
{
    for (int i=0; i<num_regions; i++) {
        if (i==0)
            ConnectSensoryRegion(false);
        else
            regions[i]->setlower(regions[i-1]);
        regions[i]->InitializeProximalDendrites();
        if (i==num_regions-1)
            regions[i]->sethigher(NULL);
        else
            regions[i]->sethigher(regions[i+1]);
    }
}

/*
 * Set the input of the first Htm region as the sensory region. refresh denotes
 * whether or not to update the pointers in the region's columns with addresses
 * of new SensoryInput objects. This is used at times when the synapses are
 * already initialized, and just need to point to new SensoryInput objects.
 */
void Htm::ConnectSensoryRegion(bool refresh)
{
    SensoryRegion *NewPattern = ConsumePattern();

    if (NewPattern == NULL)
        return;

    regions[0]->setlower(NewPattern);
    if (refresh)
        regions[0]->RefreshLowerSynapses();
}

// Return most recently seen pattern.
SensoryRegion* Htm::CurrentPattern()
{
    return currentPattern;
}

// Get the next input pattern from the codec and return it.
SensoryRegion* Htm::ConsumePattern()
{
    currentPattern = codec->GetPattern(); // may become NULL
    return currentPattern;
}

bool Htm::FirstPattern()
{
    return codec->FirstPattern(currentPattern);
}

void Htm::ResetCodec()
{
    codec->Reset();
}

void Htm::PrintPattern(SensoryRegion *pattern)
{
    printf("Printing pattern.\n");
    SensoryInput ***input = pattern->GetInput();

    for (unsigned int i=0; i<pattern->GetWidth(); i++) {
        for (unsigned int j=0; j<pattern->GetHeight(); j++)
            printf("%d", input[i][j]->IsActive());
        printf("\n");
    }
}

int Htm::new_region(HtmRegion *reg)
{
    if (!regions)
        regions = (HtmRegion **)malloc(sizeof(HtmRegion *) * (num_regions=1));
    else
        regions = (HtmRegion **)realloc(regions, sizeof(HtmRegion *) * ++num_regions);

    regions[num_regions-1] = reg;

    return 1;
}

void Htm::CLA()
{
    //printf("Running CLA...\n");
    for (int i=0; i<num_regions; i++)
        regions[i]->CLA(Learning, allowBoosting);
    //printf("CLA complete.\n");
}

// accessors

unsigned int Htm::GetWindowWidth()
{
    return this->window_w;
}

unsigned int Htm::GetWindowHeight()
{
    return this->window_h;
}

unsigned int Htm::GetNumRegions()
{
    return this->num_regions;
}

HtmRegion** Htm::GetRegions()
{
    return this->regions;
}

char* Htm::GetCodecName()
{
    return codec->GetCodecName();
}

