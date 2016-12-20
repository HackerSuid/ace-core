#include <stdio.h>

#include "qt.h"
#include "htm.h"
#include "htmsublayer.h"
#include "sensoryregion.h"

#include <ctime>

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
    CurrentInput = new QtSensoryRegion(this, htm->CurrentPattern());
    HtmDisplay = new QtHtm(this, htm);
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
    regLabel->setText("Sublayers: ");
    regLabel->setAlignment(Qt::AlignRight);
    PretrainLayout->addWidget(regLabel, 4, 1, 1, 1);

    QLabel *regVal = new QLabel();
    snprintf(regstr, sizeof(regstr), "%d", htm->GetNumSublayers());
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

    // groupbox for sensory input.
    sensoryGroup = new QGroupBox("sensory input");
    sensoryGroup->setObjectName("RegionId");
    QGridLayout *sensoryGrid = CurrentInput->SensoryUnitGrid();
    sensoryGroup->setLayout(sensoryGrid);

    // groupbox for motor input.
    motorGroup = new QGroupBox("motor input");
    motorGroup->setObjectName("RegionId");
    QGridLayout *motorGrid = CurrentInput->MotorUnitGrid();
    motorGroup->setLayout(motorGrid);

    // Interactive controls
    QWidget *controls = new QWidget;
    controls->setStyleSheet("color: white;");
    QGridLayout *controlsLayout = new QGridLayout;
    controlsLayout->setHorizontalSpacing(1);
    controlsLayout->setVerticalSpacing(3);

    QLabel *predCompWindowLab = new QLabel("Prediction Comprehension  ");
    QLabel *predSpecWindowLab = new QLabel("Prediction Specificity");
    predCompWindowVal = new QLabel(
        QString::number(
            HtmDisplay->PredictionComprehensionMetric(),
            'e', 2
        )
    );
    predSpecWindowVal = new QLabel(
        QString::number(
            HtmDisplay->PredictionSpecificityMetric(),
            'e', 2
        )
    );
    predCompWindowLab->setFont(QFont("Arial", 14, QFont::Bold));
    predSpecWindowLab->setFont(QFont("Arial", 14, QFont::Bold));
    predCompWindowVal->setFont(QFont("Arial", 14, QFont::Bold));
    predSpecWindowVal->setFont(QFont("Arial", 14, QFont::Bold));

/*
    controlsLayout->addWidget(predCompWindowLab, 0, 0);
    controlsLayout->addWidget(predCompWindowVal, 0, 1);
    controlsLayout->addWidget(predSpecWindowLab, 1, 0);
    controlsLayout->addWidget(predSpecWindowVal, 1, 1);
*/

    //controlsLayout->setRowStretch(2, 4);

    QLabel *singPattLab = new QLabel("Single pattern");
    TrainSingPattButton = new QPushButton("consume");
    TrainSingPattButton->setStyleSheet("color: black;");
    controlsLayout->addWidget(singPattLab, 0, 0);
    controlsLayout->addWidget(TrainSingPattButton, 0, 1);

    QLabel *singProgLab = new QLabel("Single program");
    TrainSingProgButton = new QPushButton("execute");
    TrainSingProgButton->setStyleSheet("color: black;");
    //controlsLayout->addWidget(singProgLab, 1, 0);
    //controlsLayout->addWidget(TrainSingProgButton, 1, 1);

    QLabel *VarLab = new QLabel("Variable program: ");
    VarEdit = new QLineEdit();
    VarEdit->setStyleSheet("color: black");
    TrainVarButton = new QPushButton("initiate");
    TrainVarButton->setStyleSheet("color: black;");
    controlsLayout->addWidget(VarLab, 1, 0);
    controlsLayout->addWidget(VarEdit, 1, 1);
    controlsLayout->addWidget(TrainVarButton, 2, 1);

    controlsLayout->setRowStretch(4, 4);
    controls->setLayout(controlsLayout);

    // groupbox for htm object details.
    objHtm = new QGroupBox("object details");
    objHtm->setStyleSheet("color: white;");

    // groupbox for htm region sublayer.
    htmGroup = new QGroupBox("sublayer 0");
    htmGroup->setObjectName("RegionId");
    QGridLayout *htmGrid = HtmDisplay->UnitGrid(objHtm);
    // connect first Htm region grid units to sensory input grid units.
    //HtmDisplay->SetQtSynapses(inputGrid);
    htmGroup->setLayout(htmGrid);

    TrainingLayout->addWidget(sensoryGroup, 0, 0, 1, 1);
    TrainingLayout->addWidget(motorGroup, 1, 0, 1, 1);
    //TrainingLayout->addWidget(objHtm, 1, 0, 1, 1);
    TrainingLayout->addWidget(controls, 2, 0, 1, 1);
    TrainingLayout->addWidget(htmGroup, 0, 1, 3, 1);

    TrainingLayout->setColumnStretch(0, 30);
    TrainingLayout->setColumnStretch(1, 100);
    TrainingLayout->setRowStretch(0, 2);
    TrainingLayout->setRowStretch(1, 8);
    TrainingLayout->setRowStretch(2, 8);
    //TrainingLayout->setRowStretch(3, 1);

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

void QtFront::UpdateQtDisplay()
{
    UpdateInputDisplay(CurrentInput->GetPattern());
    UpdateHtmDisplay();
    predCompWindowVal->setText(
        QString::number(
            HtmDisplay->PredictionComprehensionMetric(),
            'e', 2
        )
    );
    predSpecWindowVal->setText(
        QString::number(
            HtmDisplay->PredictionSpecificityMetric(),
            'e', 2
        )
    );
    this->repaint();
}

/*
 * Update the QtSensoryRegion grid display to reflect the current input
 * pattern by changing the brush color of the QtUnits.
 */
void QtFront::UpdateInputDisplay(SensoryRegion *newPattern)
{
    QGridLayout *currSensoryGrid =
        (QGridLayout *)sensoryGroup->layout();
    QGridLayout *newSensoryGrid =
        CurrentInput->SensoryUnitGrid();
    int w = newPattern->GetWidth();
    int h = newPattern->GetHeight();

    SensoryRegion *mp = newPattern->GetMotorPattern();
    QGridLayout *currMotorGrid =
        (QGridLayout *)motorGroup->layout();
    QGridLayout *newMotorGrid =
        CurrentInput->MotorUnitGrid();

    int r, g, b;
    QColor rgb;
    QtUnit *oldqtunit = NULL, *newqtunit = NULL;

/*
 The old way of updating the sensory input unit grid. This had very,
 very poor performance, so I am leaving the code here as a reminder.

    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            // getting the qlayout item with itemAtPosition has very
            // poor performance.
            qtunit = (QtUnit *)newSensoryGrid->itemAtPosition(i, j)->widget();
            rgb = qtunit->getBrushColor();
            rgb.getRgb(&r, &g, &b);
            qtunit = (QtUnit *)currSensoryGrid->itemAtPosition(i, j)->widget();
            qtunit->setBrushColor(QColor(r, g, b));
        }
    }
*/

    /* sensory input display grid */
    for (int i=0; i<newSensoryGrid->count(); i++) {
        int row, col, rs, cs;
        newSensoryGrid->getItemPosition(i, &row, &col, &rs, &cs);
        newqtunit = (QtUnit *)newSensoryGrid->itemAt(i)->widget();
        rgb = newqtunit->getBrushColor();
        rgb.getRgb(&r, &g, &b);
        oldqtunit = (QtUnit *)currSensoryGrid->itemAt(i)->widget();
        oldqtunit->setBrushColor(QColor(r, g, b));
    }
    /* motor input display grid */
    for (int i=0; i<newMotorGrid->count(); i++) {
        int row, col, rs, cs;
        newMotorGrid->getItemPosition(i, &row, &col, &rs, &cs);
        newqtunit = (QtUnit *)newMotorGrid->itemAt(i)->widget();
        rgb = newqtunit->getBrushColor();
        rgb.getRgb(&r, &g, &b);
        oldqtunit = (QtUnit *)currMotorGrid->itemAt(i)->widget();
        oldqtunit->setBrushColor(QColor(r, g, b));
        oldqtunit->SetActive(newqtunit->IsActive());
    }
}

/*
 * Update the QtHtmSublayer grid display after running the CLA.
 */
void QtFront::UpdateHtmDisplay()
{
    QGridLayout *currGrid = (QGridLayout *)htmGroup->layout();
    QGridLayout *newHtmGrid = HtmDisplay->UnitGrid(objHtm); 
    HtmSublayer **sublayers = htm->GetSublayers();
    int h = sublayers[0]->GetHeight();
    int w = sublayers[0]->GetWidth();
    int d = sublayers[0]->GetDepth();
    QtUnit *colUnit = NULL;
    QList<QtCell *> qtCells;
    //QGridLayout *cellGrid = NULL;
    QColor rgb[d+1];
    int r[d+1], g[d+1], b[d+1];

    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            colUnit = (QtUnit *)newHtmGrid->itemAtPosition(i, j)->widget();
            //cellGrid = colUnit->GetCellGrid();
            rgb[0] = colUnit->getBrushColor();
            rgb[0].getRgb(&r[0], &g[0], &b[0]);
            qtCells = colUnit->findChildren<QtCell *>();
            for (unsigned int c=1; c<=d; c++) {
                rgb[c] = qtCells.at(c-1)->getBrushColor();
                rgb[c].getRgb(&r[c], &g[c], &b[c]);
            }
            colUnit =
                (QtUnit *)currGrid->itemAtPosition(i, j)->widget();
            colUnit->setBrushColor(QColor(r[0], g[0], b[0]));
            qtCells = colUnit->findChildren<QtCell *>();
            for (unsigned int c=1; c<=d; c++)
                qtCells.at(c-1)->setBrushColor(
                    QColor(r[c], g[c], b[c])
                );
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
    connect(TrainSingPattButton, SIGNAL(clicked()), this, SLOT(RunSinglePattern()));
    connect(TrainSingProgButton, SIGNAL(clicked()), this, SLOT(RunSingleProgram()));
    connect(TrainVarButton, SIGNAL(clicked()), this, SLOT(RunVariableProgram()));

    // Analysis actions (gnuplot)
    plotPredCompAction = new QAction("Plot prediction comprehension", this);
    connect(
        plotPredCompAction,
        SIGNAL(triggered()),
        this,
        SLOT(PlotPredCompGraph())
    );
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

    analysisMenu = menuBar->addMenu("Analysis");
    analysisMenu->addAction(plotPredCompAction);
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

// Consume the next pattern of the program.
void QtFront::RunSinglePattern()
{
    if (!Run())
        return;

    // update the various Qt displays
    UpdateQtDisplay();
}

// Execute the program once and consume all the patterns.
void QtFront::RunSingleProgram()
{
    while (Run())
        ;

    UpdateQtDisplay();
}

// Initiate a series of program executions.
void QtFront::RunVariableProgram()
{
    int n = VarEdit->text().toInt();
    for (int i=0; i<n; i++) {
        while (Run())
            ;
    }

    UpdateQtDisplay();
}

int QtFront::Run()
{
    SensoryRegion *pattern;

    pattern = htm->CurrentPattern();
    if (pattern == NULL) {
        htm->ResetCodec();
        htm->ConnectSubcorticalInput(true);
        return 0;
    }
    htm->PushNextClaInput();

    CurrentInput = new QtSensoryRegion(this, pattern);
    HtmDisplay = new QtHtm(this, htm);
    return 1;
}

void QtFront::PlotPredCompGraph()
{
    htm->GeneratePredCompX11Gnuplot();
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

