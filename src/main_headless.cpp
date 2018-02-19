#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include <sys/ptrace.h>

#include "htm.h"
#include "htmsublayer.h"
#include "sensoryregion.h"

int Run(Htm *htm);
void RunProgram(Htm *htm);

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "[error] must specify configuration file.\n");
        exit(-1);
    }
    // seed the rng first thing.
    srand(time(NULL)*getpid());
    // Instantiate an Htm: initializes a handful of member variables.
    Htm *htm = new Htm;
    // Initialize the Htm from XML config and create/init codecs.
    htm->InitHtm(argv[1]);

    printf("[MAIN] Initiating runs.\n");
    for (int i=0; i<100; i++) {
        printf("[MAIN] iteration %d\n", i);
        RunProgram(htm);
    }

    return 0;
}

int Run(Htm *htm)
{
    SensoryRegion *pattern;

    htm->SendInputThroughLayers();

    pattern = htm->CurrentPattern();
    if (pattern == NULL) {
        htm->ReloadCodecTarget();
        htm->ConnectSubcorticalInput(true);
        return 0;
    }

    return 1;
}

void RunProgram(Htm *htm)
{
    while (Run(htm))
        ;
}

