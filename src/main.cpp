#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <sys/ptrace.h>

#include "qt.h"
#include "htm.h"

// Export the QApplication object pointer so the instantiated Qt classes can
// reference it after it's been instantiated itself.
QApplication *app;

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "[error] must specify configuration file.\n");
        exit(-1);
    }
    // seed the rng first thing.
    srand(time(NULL)*getpid());
    // instantiate the QApplication for use by the Qt library.
    app = new QApplication(argc, argv);
    // Instantiate an Htm: initializes a handful of member variables.
    Htm *htm = new Htm;
    // Initialize the Htm from XML config and create/init codecs.
    htm->InitHtm(argv[1]);
    // Instantiate a QtFront object to visualize the Htm.
    QtFront *display = new QtFront(htm);
    // Load the first input pattern and prepare initialize the Htm Qt
    // members.
    display->LoadQt();
    // Transfer control to Qt immediately.
    app->exec();
    // We're done when Qt exits.
    return 0;
}

