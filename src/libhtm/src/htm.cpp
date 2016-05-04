#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <rapidxml/rapidxml.hpp>

#include "htm.h"
#include "htmsublayer.h"
#include "sensoryregion.h"
#include "sensoryinput.h"
#include "codec_base.h"
#include "util.h"

Htm::Htm()
{
    sublayers = NULL;
    num_sublayers = 0;
    Learning = false;
    currentPattern = NULL;
}

Htm::~Htm()
{
}

void Htm::InitHtm()
{
    printf("Loading config: %s\n", LOCAL_XML_CONF_PATH);
    // initialization of Htm regions and columns.
    if (!LoadXmlConfig(LOCAL_XML_CONF_PATH))
        abort();
    printf("Creating and initializing codec...\n");
    // instantiation of the codec.
    if (!(codec = Codec::Instantiate()))
        abort();
    // initialization of the codec.
    if (!codec->Init(target_path))
        abort();
    // initialization and linkage of the regions.
    printf("Connecting cortical sublayers to sensory stream...\n");
    this->ConnectHeirarchy();
    printf("...complete.\n");
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
        fprintf(stderr, "target missing from configuration file.\n");
        return false;
    }
    window_w = atoi(HtmNode->first_attribute("WinWidth")->value());
    window_h = atoi(HtmNode->first_attribute("WinHeight")->value());
    allowBoosting = atob(HtmNode->first_attribute("allowBoosting")->value());
    // Add regions to the Htm
    rapidxml::xml_node<> *sublayer_node = HtmNode->first_node("Sublayer");
    if (!sublayer_node) {
        fprintf(stderr, "No sublayers found in configuration!\n");
        return false;
    }
    do {
        unsigned int h = atoi(sublayer_node->first_attribute("height")->value());
        unsigned int w = atoi(sublayer_node->first_attribute("width")->value());
        unsigned int cpc = atoi(
            sublayer_node->first_attribute("cellsPerCol")->value()
        );
        //printf("%d columns: %d x %d.\n", h*w, h, w);
        HtmSublayer *curr = new HtmSublayer(h, w, cpc, this);

        rapidxml::xml_node<> *cols = sublayer_node->first_node("Columns");
        float rfsz = atof(cols->first_attribute("recFieldSz")->value());
        float localActivity = atof(
            cols->first_attribute("localActivity")->value()
        );
        float columnComplexity = atof(
            cols->first_attribute("columnComplexity")->value()
        );
        bool highTier = atob(cols->first_attribute("highTier")->value());
        int activityCycleWindow = atoi(
            cols->first_attribute("activityCycleWindow")->value()
        );
        curr->AllocateColumns(
            rfsz,
            localActivity,
            columnComplexity,
            highTier,
            activityCycleWindow
        );

        this->NewSublayer(curr);
    } while ((sublayer_node = sublayer_node->next_sibling("Region")));

    close(xmlfd);
    return true;
}

// Set lower/higher pointers and initialize potential synapses.
void Htm::ConnectHeirarchy()
{
    for (int i=0; i<num_sublayers; i++) {
        if (i==0)
            ConnectSensoryRegion(false);
        else
            sublayers[i]->setlower(sublayers[i-1]);
        sublayers[i]->InitializeProximalDendrites();
        if (i==num_sublayers-1)
            sublayers[i]->sethigher(NULL);
        else
            sublayers[i]->sethigher(sublayers[i+1]);
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

    sublayers[0]->setlower(NewPattern);
    if (refresh)
        sublayers[0]->RefreshLowerSynapses();
}

// Return most recently seen pattern.
SensoryRegion* Htm::CurrentPattern()
{
    return currentPattern;
}

// Get the next input pattern from the codec and return it.
SensoryRegion* Htm::ConsumePattern()
{
    currentPattern = codec->GetPattern(Learning);
    return currentPattern;
}

bool Htm::FirstPattern()
{
    return codec->FirstPattern();
}

void Htm::ResetCodec()
{
    Learning = false;
    codec->Reset();
}

void Htm::PrintPattern(SensoryRegion *pattern)
{
    printf("Printing pattern.\n");
    SensoryInput ***input = pattern->GetInput();

    for (unsigned int i=0; i<pattern->GetHeight(); i++) {
        for (unsigned int j=0; j<pattern->GetWidth(); j++)
            printf("%d", input[i][j]->IsActive());
        printf("\n");
    }
}

int Htm::NewSublayer(HtmSublayer *sublayer)
{
    if (!sublayers)
        sublayers = (HtmSublayer **)malloc(sizeof(HtmSublayer *) * (num_sublayers=1));
    else
        sublayers = (HtmSublayer **)realloc(regions, sizeof(HtmSublayer *) * ++num_sublayers);

    sublayers[num_sublayers-1] = sublayer;

    return 1;
}

void Htm::CLA()
{
    if (Learning == false)
        Learning = true;
    //printf("Running CLA...\n");
    for (int i=0; i<num_sublayers; i++)
        sublayers[i]->CLA(Learning, allowBoosting);
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

unsigned int Htm::GetNumSublayers()
{
    return this->num_sublayers;
}

HtmSublayer** Htm::GetSublayers()
{
    return this->sublayers;
}

char* Htm::GetCodecName()
{
    return codec->GetCodecName();
}

