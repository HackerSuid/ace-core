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

#define DEF_UNIT_W  30
#define DEF_UNIT_H  30

#define DEF_CELL_W  4
#define DEF_CELL_H  4

class SensoryRegion;
class Htm;
class GenericInput;

class QtHtm;
class QtSensoryRegion;
class QtUnit;

const QColor ACTIVE_COLOR = QColor(180, 180, 180);
const QColor ACTIVE_COLOR_SHOW = QColor(110, 180, 110);
const QColor INACTIVE_COLOR = QColor(90, 90, 90);
const QColor INACTIVE_COLOR_SHOW = QColor(90, 90, 170);
const QColor PREDICTED_COLOR = QColor(255, 255, 102);

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
    QGroupBox *inputGroup, *htmGroup, *objHtm;
    QPushButton *TrainNextButton, *TrainIterButton;
    QLineEdit *IterEdit;

    QWidget *activeWindow;

    QMenuBar *menuBar;
    QMenu *fileMenu, *configMenu, *execMenu, *debugMenu;
    QAction *openAction, *saveAction, *exitAction;
    QAction *showConfAction;
    QAction *trainAction;

    // Qt classes to display Htm features
    QtSensoryRegion *CurrentInput;
    QtHtm *HtmDisplay;
private slots:
    void ShowPretrainWidget();
    void ShowTrainingWidget();
    void RunSingleCLA();
    void RunVariableCLA();
    int RunCLA();
protected:
    // override virtual methods from QWidget
    void paintEvent(QPaintEvent *e);
    void closeEvent(QCloseEvent *e);
public:
    QtFront(Htm *htm);
    ~QtFront();
    void LoadQt();
    void UpdateInputDisplay(SensoryRegion *newPattern);
    void UpdateHtmDisplay();
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
    QGridLayout *unitGrid;
public:
    QtSensoryRegion(QWidget *parent, SensoryRegion *patt);
    ~QtSensoryRegion();
    QGridLayout* UnitGrid();
    SensoryRegion* GetPattern();
};

// separate widget for htm for different slots and signals than input.
class QtHtm : public QWidget
{
Q_OBJECT
private:
    QWidget *parent;
    Htm *htm;
    QGridLayout *unitGrid;
public:
    QtHtm(QWidget *parent, Htm *htm);
    ~QtHtm();
    QGridLayout* UnitGrid(QGroupBox *objHtm);
    void SetQtSynapses(QGridLayout *inputGrid);
    void DrawCell(QPainter *p, unsigned int x, unsigned int y);
};

class QtUnit : public QWidget
{
Q_OBJECT
public:
    QtUnit(GenericInput *GridNode, QGroupBox *objHtm, int c, int w=DEF_UNIT_W, int h=DEF_UNIT_H);
    ~QtUnit();
    void CreateActions();
    QSize sizeHint() const;
    void setBrushColor(const QColor &newColor);
    QColor getBrushColor() const;
    QGridLayout* GetCellGrid();
    void SetClickable(bool flag);
    bool IsClickable();
protected:
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    // context menu members
    QAction *showConnections, *hideConnections;
    // cell layout members
    QGridLayout *CellGrid;
    QColor brushColor;
    int sizeW, sizeH;
    int cells;
    GenericInput *node;
    QGroupBox *objDetail;
    // other members
    bool isClickable;
    bool toggled;

    void _ToggleConnections(bool flag);
private slots:
    void ShowConnections();
    void HideConnections();
};

class QtCell : public QWidget
{
//Q_OBJECT
public:
    QtCell(Cell *cell, int w=DEF_CELL_W, int h=DEF_CELL_H);
    ~QtCell();
    QSize sizeHint() const;
    void setBrushColor(const QColor &newColor);
    QColor getBrushColor() const;
protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    QColor brushColor;
    int sizeW, sizeH;
    Cell *cell;
};

#endif
