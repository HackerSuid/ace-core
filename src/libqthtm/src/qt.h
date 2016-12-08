#ifndef QT_H_
#define QT_H_

#include <QtGui/QApplication>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtCore/QObject>
#include <QtGui/QLabel>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>
#include <QtGui/QScrollArea>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QPainter>

#include "cell.h"

#define DISPLAY_X   10
#define DISPLAY_Y   100
#define CELL_DIA    8.0
#define COL_DIA     ((CELL_DIA*2.5)+CELL_DIA)

#define DEF_UNIT_W  16
#define DEF_UNIT_H  16

#define DEF_CELL_W  5
#define DEF_CELL_H  5

class SensoryRegion;
class Htm;
class GenericInput;

class QtHtm;
class QtSensoryRegion;
class QtUnit;

/* dark white/very light grey */
const QColor ACTIVE_COLOR = QColor(180, 180, 180);
/* medium grey */
const QColor INACTIVE_COLOR = QColor(90, 90, 90);
/* bright yellow */
const QColor PREDICTED_COLOR = QColor(255, 255, 102);
/* some sort of red */
const QColor HIGHLIGHT_COLOR = QColor(0xFF, 0xAA, 0x33);

extern QApplication *app;

class QtFront : public QWidget
{
Q_OBJECT
private:
    Htm *htm;

    // Window organization members
    QVBoxLayout *DefaultLayout;
    // pretraining window widgets
    QWidget *PretrainWindow;
    QGridLayout *PretrainLayout;
    // training window widgets
    QWidget *TrainingWindow;
    QGridLayout *TrainingLayout;
    QGroupBox *sensoryGroup, *motorGroup, *htmGroup, *objHtm;
    QLabel *predCompWindowVal, *predSpecWindowVal;
    QPushButton *TrainSingPattButton, *TrainSingProgButton, *TrainVarButton;
    QLineEdit *VarEdit;

    QWidget *activeWindow;

    QMenuBar *menuBar;
    QMenu *fileMenu, *configMenu, *execMenu, *analysisMenu;
    QAction *openAction, *saveAction, *exitAction;
    QAction *showConfAction;
    QAction *trainAction;
    QAction *plotPredCompAction;

    // Qt classes to display Htm features
    QtSensoryRegion *CurrentInput;
    QtHtm *HtmDisplay;
private slots:
    void ShowPretrainWidget();
    void ShowTrainingWidget();
    void RunSinglePattern();
    void RunSingleProgram();
    void RunVariableProgram();
    int Run();
    void PlotPredCompGraph();
protected:
    // override virtual methods from QWidget
    void paintEvent(QPaintEvent *e);
    void closeEvent(QCloseEvent *e);
public:
    QtFront(Htm *htm);
    ~QtFront();
    void LoadQt();
    void UpdateQtDisplay();
    void UpdateInputDisplay(SensoryRegion *newPattern);
    void UpdateHtmDisplay();
    QtSensoryRegion* GetCurrentInputDisplay() { return CurrentInput; }
    void CreateActions();
    void CreateMenuBar();
    void CreatePretrainWidget();
    void CreateTrainingWidget();
    void SetActiveWindow(QWidget *w);
    QWidget* ActiveWindow();
};

class QtSensoryRegion : public QWidget
{
Q_OBJECT
private:
    QWidget *parent;
    SensoryRegion *pattern;
    QGridLayout *sensoryUnitGrid, *motorUnitGrid;
public:
    QtSensoryRegion(QWidget *parent, SensoryRegion *patt);
    ~QtSensoryRegion();
    QGridLayout* SensoryUnitGrid();
    QGridLayout* MotorUnitGrid();
    SensoryRegion* GetPattern();
};

// separate widget for htm for different slots and signals than input.
class QtHtm : public QWidget
{
Q_OBJECT
private:
    QWidget *parent;
    Htm *htm;
    QGridLayout *htmUnitGrid;

    float SlidingWindowAvg(std::list<float> window);
public:
    QtHtm(QWidget *parent, Htm *htm);
    ~QtHtm();
    QGridLayout* UnitGrid(QGroupBox *objHtm);
    float PredictionComprehensionMetric();
    float PredictionSpecificityMetric();
    void SetQtSynapses(QGridLayout *inputGrid);
    void DrawCell(QPainter *p, unsigned int x, unsigned int y);
};

class QtUnit : public QWidget
{
Q_OBJECT
public:
    QtUnit(
        GenericInput *GridNode,
        QGroupBox *objHtm,
        QGridLayout *htmGrid,
        QGridLayout *sensoryGrid,
        QGridLayout *motorGrid,
        int c, int w=DEF_UNIT_W, int h=DEF_UNIT_H
    );
    ~QtUnit();
    void CreateActions();
    QSize sizeHint() const;
    void setBrushColor(const QColor &newColor);
    QColor getBrushColor() const;
    void SaveBrushColor() { prevBrushColor = brushColor; }
    void RestoreBrushColor() { brushColor = prevBrushColor; }
    QGridLayout* GetCellGrid();
    void SetClickable(bool flag);
    bool IsClickable();
protected:
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    QGridLayout *htmGrid, *sensoryGrid, *motorGrid;
    // context menu members
    QAction *showProximalConnections, *hideProximalConnections;
    // cell layout members
    QGridLayout *CellGrid;
    QColor brushColor, prevBrushColor;
    int sizeW, sizeH;
    int cells;
    GenericInput *node;
    QGroupBox *objDetail;
    // other members
    bool isClickable;
    bool toggled;

    void _ToggleProximalConnections(bool flag);
private slots:
    void ShowProximalConnections();
    void HideProximalConnections();
};

class QtCell : public QWidget
{
Q_OBJECT
public:
    QtCell(
        QWidget *parent,
        Cell *cell,
        QGridLayout *htmGrid,
        QGridLayout *sensoryGrid,
        QGridLayout *motorGrid,
        int w=DEF_CELL_W,
        int h=DEF_CELL_H
    );
    ~QtCell();
    void CreateActions();
    QSize sizeHint() const;
    void setBrushColor(const QColor &newColor);
    QColor getBrushColor() const;
protected:
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    QAction *showDistalConnections, *hideDistalConnections;
    QColor brushColor;
    int sizeW, sizeH;
    Cell *cell;
    QGridLayout *htmGrid, *sensoryGrid, *motorGrid;
    bool toggled;

    void _ToggleDistalConnections(bool flag);
private slots:
    void ShowDistalConnections();
    void HideDistalConnections();
};

#endif

