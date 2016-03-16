#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "qt.h"
#include "htm.h"

// Export the QApplication object pointer so the instantiated Qt classes can
// reference it after it's been instantiated itself.
QApplication *app;

/*
 * in the debug build, main() starts and maintains the flow of program
 * execution through Qt events so that control can be more closely caused
 * and seen by the user.
 */
int main(int argc, char **argv)
{
    // seed the rng first thing.
    srand(time(NULL)*getpid());
    // instantiate the QApplication for use by the Qt library.
    app = new QApplication(argc, argv);
    // Instantiate an Htm
    Htm *htm = new Htm;
    // Initialize the Htm
    htm->InitHtm();
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

