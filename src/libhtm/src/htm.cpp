#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <rapidxml/rapidxml.hpp>

#include "htm.h"
#include "htmsublayer.h"
#include "sensoryregion.h"
#include "sensoryinput.h"
#include "codec_base.h"
#include "util.h"
#include "pooling_layer.h"

Htm::Htm()
{
    sublayers = NULL;
    num_sublayers = 0;
    Learning = false;
    currentPattern = NULL;

    poolingLayer = NULL;
}

Htm::~Htm()
{
}

void Htm::InitHtm(const char *config_file_path)
{
    printf("[*] Loading HTM configuration: %s\n", config_file_path);
    // initialization of Htm regions and columns.
    if (!LoadXmlConfig(config_file_path))
        abort();
    printf("[*] Creating and initializing codec.\n");
    // instantiation of the codec.
    if (!(codec = Codec::Instantiate()))
        abort();
    // initialization of the codec.
    if (!codec->Init(target_path, sublayers[0], locCodecSz, locCodecBits))
        abort();
    // initialization and linkage of the regions.
    printf("[*] Connecting cortical sublayers to sensory stream.\n");
    this->ConnectHeirarchy();
    printf("[htm] Initialized.\n");
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
    char *logfile = NULL;
    rapidxml::xml_attribute<> *lognode =
        HtmNode->first_attribute("output_log");
    if (lognode) {
        logfile = lognode->value();
        printf("Redirecting stdout and stderr to %s\n", logfile);
        if ((logfd = open(logfile, O_RDWR|O_CREAT|O_TRUNC)) < 0)
            perror("open() failed: ");
        if (dup2(logfd, STDOUT_FILENO) < 0)
            perror("dup2() failed for STDOUT: ");
        if (dup2(logfd, STDERR_FILENO) < 0)
            perror("dup2() failed for STDERR: ");
        printf("[*] HTM log initialized: %s\n", logfile);
    }
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
        rapidxml::xml_attribute<> *smotor_attr =
            sublayer_node->first_attribute("sensorimotor");
        bool sensorimotor = true;
        if (smotor_attr) {
            sensorimotor = atob(
                sublayer_node->first_attribute("sensorimotor")->value()
            );
            rapidxml::xml_attribute<> *lpattsz_attr =
                sublayer_node->first_attribute(
                    "locationPatternSz"
            );
            rapidxml::xml_attribute<> *lpattbits_attr =
                sublayer_node->first_attribute(
                    "locationPatternBits"
            );
            if (!lpattsz_attr || !lpattbits_attr) {
                fprintf(stderr, "Location pattern sz & bits are required for SMI.\n");
                abort();
            }
            locCodecSz = atoi(lpattsz_attr->value());
            locCodecBits = atoi(lpattbits_attr->value());
        }
        //printf("%d columns: %d x %d.\n", h*w, h, w);
        //printf("[htm] sensorimotor flag: %d\n", sensorimotor);
        HtmSublayer *curr = new HtmSublayer(h, w, cpc, this, sensorimotor);

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

    //poolingLayer = new PoolingLayer(24, 24, 0, 8, this, sublayers[0]);

    close(xmlfd);
    return true;
}

// Set lower/higher pointers and initialize potential synapses.
void Htm::ConnectHeirarchy()
{
    for (int i=0; i<num_sublayers; i++) {
        if (i==0)
            ConnectSubcorticalInput(false);
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
void Htm::ConnectSubcorticalInput(bool refresh)
{
    SensoryRegion *NewPattern = ConsumePattern();

    if (NewPattern == NULL)
        return;

    sublayers[0]->setlower(NewPattern);
    if (refresh) {
        sublayers[0]->RefreshLowerSynapses();
    }
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

void Htm::ReloadCodecTarget()
{
    Learning = false;
    codec->ReloadTarget();
}

void Htm::PrintPattern(SensoryRegion *pattern)
{
    printf("Printing pattern.\n");
    SensoryInput ***input = pattern->GetInput();

    printf("\t%d x %d\n", pattern->GetHeight(), pattern->GetWidth());
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

// the first subcortical input is loaded during initialization, so load the
// next one at the end.
void Htm::SendInputThroughLayers()
{
    if (Learning == false)
        Learning = true;
    printf("[*] Sending current input pattern through layers.\n");
    for (int i=0; i<num_sublayers; i++)
        sublayers[i]->ComputeLayerStateFromInput(
            Learning, allowBoosting);
//    poolingLayer->PoolInputColumns();
    printf("[*] Consuming next input pattern\n");
    ConnectSubcorticalInput(true);
}
/*
 * Generates a gnuplot data file containing the data points for
 * prediction comprehension from each timestep for the length of
 * the sliding average window.
 * The data file is then input to a generated gnuplot command file
 * that contains the plotting commands.
 */
void Htm::GeneratePredCompX11Gnuplot()
{
    printf(
        "[*] Generating gnuplot graph for "
        "prediction comprehension\n"
    );
    std::list<float> predCompWindow =
        sublayers[0]->GetPredictionComprehensionWindow();
    int cmdfd = open("/tmp/htm_gnuplot_cmd", O_RDWR|O_CREAT|O_TRUNC);
    int datfd = open("/tmp/htm_gnuplot_dat", O_RDWR|O_CREAT|O_TRUNC);

    if (cmdfd < 0) {
        perror("\t[*] Failed to open command file: ");
        return;
    }

    if (datfd < 0) {
        perror("\t[*] Failed to open data file: ");
        return;
    }

    /* create the data file. */
    char data[16];
    std::list<float>::iterator it;
    int i;
    for (it=predCompWindow.begin(),i=0; it!=predCompWindow.end(); it++,i++) {
        /* # id value */
        memset(data, 0, sizeof(data));
        snprintf(data, sizeof(data), "%d", i);
        write(datfd, data, strlen(data));
        write(datfd, " ", 1);
        memset(data, 0, sizeof(data));
        snprintf(data, sizeof(data), "%2.2f", (float)(*it));
        write(datfd, data, strlen(data));
        write(datfd, "\n", 1);
    }
    close(datfd);

    /* create the script of gnuplot commands */
    unsigned char *gnuplotscript =
        (unsigned char *)"#!/usr/local/bin/gnuplot\n" \
        "set terminal x11\n" \
        "set style line 1 lc rgb '#0060ad' lt 1 lw 2\n" \
        "set yrange [0.000:1.2]\n" \
        "set title 'Prediction Comprehension'\n" \
        "set xlabel 'timestep'\n" \
        "set ylabel 'comp. %'\n" \
        "plot '/tmp/htm_gnuplot_dat' with linespoints ls 1\n";
    write(cmdfd, (void *)gnuplotscript, strlen((const char *)gnuplotscript)); 
    close(cmdfd);

    system("$(which gnuplot) -p /tmp/htm_gnuplot_cmd");

/*
    pid_t child;
    int status = 0;

    char *argv[] = { "gnuplot", "-p", "/tmp/htm_gnuplot_cmd", 0 };

    if ((child=fork()) == 0) {
        printf("child: executing gnuplot\n");
        execve(argv[0], &argv[0], NULL);
    } else if (child > 0)
        wait(&status);
        if (WIFEXITED(status)) {
            printf("child terminated normally\n");
        }
    else {
        perror("fork()");
    }
*/
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

