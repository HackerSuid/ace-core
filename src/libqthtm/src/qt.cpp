#include <stdio.h>

#include "qt.h"
#include "htm.h"
#include "htmregion.h"
#include "sensoryregion.h"

QtFront::QtFront(Htm *htm)
{
    this->htm = htm;
    activeWindow = NULL;

    DefaultLayout = new QVBoxLayout();
    DefaultLayout->setMargin(0);
    setStyleSheet(
        "QWidget#MainWidget {"
            "background-color: rgb(25, 25, 25);"
        "}"
        "QGroupBox {"
            "border: 2px solid gray;"
            "border-radius: 3px;"
            "margin-top: 0.5em;"
        "}"
        "QGroupBox::title {"
            "subcontrol-origin: margin;"
            "left: 10px;"
            "padding: 0px 3px 0px 3px;"
            "color: white;"
        "}"
        "QGroupBox#RegionId {"
            "background-color: rgb(25, 25, 25);"
        "}"
    );
}

QtFront::~QtFront()
{
}

void QtFront::LoadQt()
{
    // Set the configured window size.
    setFixedSize(htm->GetWindowWidth(), htm->GetWindowHeight());
    // Initialize the Qt display objects.
    HtmDisplay = new QtHtm(this, htm);
    CurrentInput = new QtSensoryRegion(this, htm->CurrentPattern());
    // Initialize the widgets.
    CreatePretrainWidget();
    CreateTrainingWidget();
    CreateActions();
    CreateMenuBar();
    // Initialize the layout.
    DefaultLayout->setMenuBar(menuBar);
    setLayout(DefaultLayout);
    // Display the default window widget.
    ShowTrainingWidget();
    // Display the UI from the application.
    this->show();
}

void QtFront::CreatePretrainWidget()
{
    char regstr[128];

    PretrainWindow = new QWidget();
    PretrainWindow->setFixedSize(this->width(), this->height());
    PretrainWindow->setObjectName("MainWidget");
    PretrainLayout = new QGridLayout();

    PretrainLayout->setSpacing(0);

    PretrainLayout->setRowStretch(0, 4);
    PretrainLayout->setRowStretch(1, 2);
    PretrainLayout->setRowStretch(2, 1);
    PretrainLayout->setRowStretch(3, 1);
    PretrainLayout->setRowStretch(4, 12);

    PretrainLayout->setColumnStretch(0, 2);
    PretrainLayout->setColumnStretch(1, 1);
    PretrainLayout->setColumnStretch(2, 1);
    PretrainLayout->setColumnStretch(3, 2);

    QLabel *msglabel = new QLabel();
    msglabel->setText("Select an Action from the menubar to begin.");
    msglabel->setAlignment(Qt::AlignCenter);
    msglabel->setFont(QFont("Arial", 14, QFont::Bold));
    PretrainLayout->addWidget(msglabel, 1, 0, 1, 4);

    QLabel *title = new QLabel();
    title->setText("HTM loaded with the following parameters:");
    title->setAlignment(Qt::AlignCenter);
    PretrainLayout->addWidget(title, 2, 1, 1, 2);

    QLabel *codecLab = new QLabel();
    codecLab->setText("Codec: ");
    codecLab->setAlignment(Qt::AlignRight);
    PretrainLayout->addWidget(codecLab, 3, 1, 1, 1);

    QLabel *codecVal = new QLabel();
    codecVal->setText(htm->GetCodecName());
    codecVal->setAlignment(Qt::AlignLeft);
    PretrainLayout->addWidget(codecVal, 3, 2, 1, 1);

    QLabel *regLabel = new QLabel();
    regLabel->setText("Regions: ");
    regLabel->setAlignment(Qt::AlignRight);
    PretrainLayout->addWidget(regLabel, 4, 1, 1, 1);

    QLabel *regVal = new QLabel();
    snprintf(regstr, sizeof(regstr), "%d", htm->GetNumRegions());
    regVal->setText(regstr);
    regVal->setAlignment(Qt::AlignLeft);
    PretrainLayout->addWidget(regVal, 4, 2, 1, 1);

    PretrainWindow->setLayout(PretrainLayout);
}

void QtFront::CreateTrainingWidget()
{
    TrainingWindow = new QWidget();
    TrainingWindow->setFixedSize(this->width(), this->height());
    TrainingWindow->setObjectName("MainWidget");
    TrainingLayout = new QGridLayout();

    TrainingLayout->setColumnStretch(0, 5);
    TrainingLayout->setColumnStretch(1, 2);
    TrainingLayout->setColumnStretch(2, 5);
    TrainingLayout->setRowStretch(0, 10);
    TrainingLayout->setRowStretch(1, 4);
    TrainingLayout->setRowStretch(2, 1);

    // groupbox for input region.
    inputGroup = new QGroupBox("input");
    inputGroup->setObjectName("RegionId");
    QGridLayout *inputGrid = CurrentInput->UnitGrid();
    inputGroup->setLayout(inputGrid);

    // trainings buttons
    QWidget *buttons = new QWidget;
    buttons->setStyleSheet("color: white;");
    QGridLayout *buttonsLayout = new QGridLayout;
    buttonsLayout->setSpacing(0);
    QLabel *nextLab = new QLabel("next");
    TrainNextButton = new QPushButton("go");
    TrainNextButton->setStyleSheet("color: black;");
    buttonsLayout->addWidget(nextLab, 0, 0);
    buttonsLayout->addWidget(TrainNextButton, 0, 1);

    QLabel *iterLab = new QLabel("iterations: ");
    IterEdit = new QLineEdit();
    IterEdit->setStyleSheet("color: black");
    TrainIterButton = new QPushButton("go");
    TrainIterButton->setStyleSheet("color: black;");
    buttonsLayout->addWidget(iterLab, 1, 0);
    buttonsLayout->addWidget(IterEdit, 1, 1);
    buttonsLayout->addWidget(TrainIterButton, 2, 1);

    buttonsLayout->setRowStretch(3, 4);
    buttons->setLayout(buttonsLayout);

    // groupbox for htm object details.
    objHtm = new QGroupBox("object details");
    objHtm->setStyleSheet("color: white;");

    // groupbox for htm region.
    htmGroup = new QGroupBox("region 0");
    htmGroup->setObjectName("RegionId");
    QGridLayout *htmGrid = HtmDisplay->UnitGrid(objHtm);
    // connect first Htm region grid units to sensory input grid units.
    HtmDisplay->SetQtSynapses(inputGrid);
    htmGroup->setLayout(htmGrid);

    TrainingLayout->addWidget(inputGroup, 0, 0, 1, 1);
    TrainingLayout->addWidget(objHtm, 1, 0, 1, 1);
    TrainingLayout->addWidget(buttons, 0, 1, 1, 1);
    TrainingLayout->addWidget(htmGroup, 0, 2, 2, 1);

    TrainingWindow->setLayout(TrainingLayout);
/*    QTabWidget *tab = new QTabWidget();
    QLabel *trainLab = new QLabel("Training tab");
    QLabel *testLab = new QLabel("Testing tab");
    tab->addTab(trainLab, "Train");
    tab->addTab(testLab, "Test");
    TrainingLayout->addWidget(tab, 0, 0);
*/
}

/*
 * Represents the screen presented to the user upon starting the Qt
 * application before any learning occurs.
 */
void QtFront::ShowPretrainWidget()
{
    if (ActiveWindow()) {
        if (ActiveWindow() == PretrainWindow)
            return;
        else {
            DefaultLayout->removeWidget(ActiveWindow());
            ActiveWindow()->hide();
        }
    }
    SetActiveWindow(PretrainWindow);
    DefaultLayout->addWidget(PretrainWindow);
    if (!PretrainWindow->isVisible())
        PretrainWindow->show();
}

void QtFront::ShowTrainingWidget()
{
    if (ActiveWindow()) {
        if (ActiveWindow() == TrainingWindow)
            return;
        else {
            DefaultLayout->removeWidget(ActiveWindow());
            ActiveWindow()->hide();
        }
    }
    SetActiveWindow(TrainingWindow);
    DefaultLayout->addWidget(TrainingWindow);
    if (!TrainingWindow->isVisible())
        TrainingWindow->show();
}

/*
 * Update the QtSensoryRegion grid display to reflect the current input
 * pattern by changing the brush color of the QtUnits.
 */
void QtFront::UpdateInputDisplay(SensoryRegion *newPattern)
{
    QGridLayout *currGrid = (QGridLayout *)inputGroup->layout();
    QGridLayout *newInputGrid = CurrentInput->UnitGrid();
    int w = newPattern->GetWidth();
    int h = newPattern->GetHeight();
    int r, g, b;
    QColor rgb;
    QtUnit *qtunit = NULL;

    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            qtunit = (QtUnit *)newInputGrid->itemAtPosition(i, j)->widget();
            rgb = qtunit->getBrushColor();
            rgb.getRgb(&r, &g, &b);
            qtunit = (QtUnit *)currGrid->itemAtPosition(i, j)->widget();
            qtunit->setBrushColor(QColor(r, g, b));
        }
    }
}

/*
 * Update the QtHtmRegion grid display after running the CLA.
 */
void QtFront::UpdateHtmDisplay()
{
    QGridLayout *currGrid = (QGridLayout *)htmGroup->layout();
    QGridLayout *newHtmGrid = HtmDisplay->UnitGrid(objHtm); 
    HtmRegion **regions = htm->GetRegions();
    int h = regions[0]->GetHeight();
    int w = regions[0]->GetWidth();
    QtUnit *colUnit = NULL;
    //QGridLayout *cellGrid = NULL;
    QColor rgb;
    int r, g, b;

    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            colUnit = (QtUnit *)newHtmGrid->itemAtPosition(i, j)->widget();
            //cellGrid = colUnit->GetCellGrid();
            rgb = colUnit->getBrushColor();
            rgb.getRgb(&r, &g, &b);
            colUnit = (QtUnit *)currGrid->itemAtPosition(i, j)->widget();
            colUnit->setBrushColor(QColor(r, g, b));
        }
    }
}

void QtFront::CreateActions()
{
    // Menu actions
    openAction = new QAction("Open", this);

    saveAction = new QAction("Save", this);

    exitAction = new QAction("Exit", this);

    showConfAction = new QAction("Show config", this);
    connect(showConfAction, SIGNAL(triggered()),
               this, SLOT(ShowPretrainWidget()));

    trainAction = new QAction("Train", this);
    connect(trainAction, SIGNAL(triggered()),
            this, SLOT(ShowTrainingWidget()));

    // Window actions
    connect(TrainNextButton, SIGNAL(clicked()), this, SLOT(RunSingleCLA()));
    connect(TrainIterButton, SIGNAL(clicked()), this, SLOT(RunVariableCLA()));
}

void QtFront::CreateMenuBar()
{
    menuBar = new QMenuBar(this);

    fileMenu = menuBar->addMenu("File");
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(exitAction);

    configMenu = menuBar->addMenu("Configure");
    configMenu->addAction(showConfAction);

    execMenu = menuBar->addMenu("Execute");
    execMenu->addAction(trainAction);

    debugMenu = menuBar->addMenu("Debug");
}

void QtFront::SetActiveWindow(QWidget *w)
{
    this->activeWindow = w;
}

QWidget* QtFront::ActiveWindow()
{
    return this->activeWindow;
}

// BEGIN QT SLOTS

void QtFront::RunSingleCLA()
{
    if (!RunCLA())
        return;

    // update the various Qt displays
    UpdateInputDisplay(CurrentInput->GetPattern());
    UpdateHtmDisplay();
    this->repaint();
}

// Iterate through all the patterns variable number of times.
void QtFront::RunVariableCLA()
{
    int n = IterEdit->text().toInt();
    for (int i=0; i<n; i++)
        while (RunCLA())
            ;
    UpdateHtmDisplay();
    this->repaint();
}

int QtFront::RunCLA()
{
    SensoryRegion *pattern;

    pattern = htm->CurrentPattern();
    if (pattern == NULL) {
        htm->ResetCodec();
        htm->ConnectSensoryRegion(true);
        return 0;
    }
    htm->CLA();

    CurrentInput = new QtSensoryRegion(this, pattern);
    HtmDisplay = new QtHtm(this, htm);
    // Connect Htm to the next pattern from the codec.
    htm->ConnectSensoryRegion(true);
    return 1;
}

// END QT SLOTS

// paintEvent() event handler is called when:
// 1) the widget window is first shown
// 2) call to QWidget::update() or QWidget::repaint()
//    - update() is blocked, repaint() is not.
void QtFront::paintEvent(QPaintEvent *e)
{
    //this->UpdateDisplay();
}

void QtFront::closeEvent(QCloseEvent *e)
{
}

