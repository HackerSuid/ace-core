#include <stdio.h>
#include <math.h>

#include "qt.h"
#include "column.h"
#include "dendritesegment.h"
#include "synapse.h"

QtUnit::QtUnit(
    GenericInput *GridNode,
    QGroupBox *objHtm,
    QGridLayout *htmGrid,
    QGridLayout *sensoryGrid,
    QGridLayout *motorGrid,
    int c, int w, int h)
{
    // don't resize on repaint.
    setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    brushColor = INACTIVE_COLOR;
    toggled = false;
    isClickable = false;

    CreateActions();

    sizeW = w;
    sizeH = h;
    node = GridNode;
    objDetail = objHtm;
    this->htmGrid = htmGrid;
    this->sensoryGrid = sensoryGrid;
    this->motorGrid = motorGrid;

    cells = c;
    if (cells) {
        std::vector<Cell *> HtmCells = ((Column *)node)->GetCells();
        for (int i=0; i<sqrt(cells); i++) {
            for (int j=0; j<sqrt(cells); j++) {
                QtCell *cell = new QtCell(
                    this,
                    HtmCells[(int)(j+(i*sqrt(cells)))],
                    htmGrid,
                    sensoryGrid,
                    motorGrid
                );
                //CellGrid->addWidget(cell, i, j);
                cell->setGeometry(
                    (j>0?DEF_CELL_W*1.25:DEF_CELL_W)/2*(j+1),
                    (i>0?DEF_CELL_H*1.25:DEF_CELL_H)/2*(i+1),
                    DEF_CELL_W, DEF_CELL_H
                );
            }
        }
        //this->setLayout(CellGrid);
    }
}

QtUnit::~QtUnit()
{
}

void QtUnit::CreateActions()
{
    showProximalConnections =
        new QAction("Show proximal connections", this);
    connect(
        showProximalConnections,
        SIGNAL(triggered()),
        this,
        SLOT(ShowProximalConnections())
    );
    hideProximalConnections =
        new QAction("Hide proximal connections", this);
    connect(
        hideProximalConnections,
        SIGNAL(triggered()),
        this,
        SLOT(HideProximalConnections())
    );
}

void QtUnit::ShowProximalConnections()
{
    _ToggleProximalConnections(true);
}

void QtUnit::HideProximalConnections()
{
    _ToggleProximalConnections(false);
}

void QtUnit::_ToggleProximalConnections(bool flag)
{
    if (sensoryGrid == NULL)
        return;
    if (toggled == flag)
        return;
    toggled = flag;
    Column *col = (Column *)node;
    DendriteSegment *segment = col->GetProximalDendriteSegment();
    std::vector<Synapse *> prox_syns = segment->GetSynapses();
    int rfs = col->GetRecFieldSz();

    //printf("rfs = %d\n", rfs);
    for (int i=0; i<rfs; i++) {
        QtUnit *src = (QtUnit *)(sensoryGrid->itemAtPosition(
            prox_syns[i]->GetY(), prox_syns[i]->GetX())->widget());
        if (flag) {
            src->SaveBrushColor();
            src->setBrushColor(HIGHLIGHT_COLOR);
        } else
            src->RestoreBrushColor();
        //printf("\t[%d] (%d, %d)\n",
            //i, prox_syns[i]->GetX(), prox_syns[i]->GetY());
    }
    ((QtSensoryRegion *)(sensoryGrid->parentWidget()))->repaint();
}

void QtUnit::SetClickable(bool flag)
{
    isClickable = flag;
}

bool QtUnit::IsClickable()
{
    return isClickable;
}

QSize QtUnit::sizeHint() const
{
    QSize sz(sizeW, sizeH);
    return sz;
}

void QtUnit::setBrushColor(const QColor &newColor)
{
    brushColor = newColor;
}

QColor QtUnit::getBrushColor() const
{
    return brushColor;
}

QGridLayout* QtUnit::GetCellGrid()
{
    return CellGrid;
}

void QtUnit::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QRect r(0, 0, sizeHint().width(), sizeHint().height());
    p.setBrush(brushColor);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap));
    p.drawRoundedRect(r, 15.0, 15.0);
}

void QtUnit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    if (!this->IsClickable())
        return;
    QVBoxLayout *objGroupLayout = new QVBoxLayout();
    QScrollArea *objScroll = new QScrollArea(objDetail);
    objScroll->setStyleSheet("background-color: rgb(25, 25, 25);");
    objScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    objScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QWidget *scrollChild = new QWidget();
    QGridLayout *objGrid = new QGridLayout(scrollChild);
    objGrid->setSizeConstraint(QLayout::SetMinAndMaxSize);

    QLayout *currentLayout = NULL;
    QLayoutItem *child = NULL;
    if (objDetail) {
        if ((currentLayout = objDetail->layout())) {
            while ((child = currentLayout->takeAt(0)) != 0)
                delete child->widget();
            delete currentLayout;
        }
        
    }

    objGroupLayout->addWidget(objScroll);

    QLabel *columnPosLab = new QLabel("position: ");
    int colx = ((Column *)node)->GetX();
    int coly = ((Column *)node)->GetY();
    char posbuf[32];
    snprintf(posbuf, sizeof(posbuf), "(%d,%d)", colx, coly);
    QLabel *columnPosVal = new QLabel(posbuf);
    objGrid->addWidget(columnPosLab, 0, 0);
    objGrid->addWidget(columnPosVal, 0, 1);
    QLabel *overlapLab = new QLabel("overlap: ");
    QLabel *overlapVal = new QLabel(QString::number(((Column *)node)->GetOverlap()));
    objGrid->addWidget(overlapLab, 1, 0);
    objGrid->addWidget(overlapVal, 1, 1);

    QLabel *synapsesLab = new QLabel("synaptic details");
    objGrid->addWidget(synapsesLab, 2, 0, 1, 3);
    QLabel *idx = new QLabel("Idx");
    QLabel *coord = new QLabel("Coord");
    QLabel *firing = new QLabel("Firing");
    QLabel *perm = new QLabel("Perm");
    objGrid->addWidget(idx, 3, 0);
    objGrid->addWidget(coord, 3, 1);
    objGrid->addWidget(firing, 3, 2);
    objGrid->addWidget(perm, 3, 3);

    Column *col = (Column *)node;
    DendriteSegment *segment = col->GetProximalDendriteSegment();
    std::vector<Synapse *> syns  = segment->GetSynapses();

    for (int i=0; i<col->GetRecFieldSz(); i++) {
        char synCoordStr[32];
        memset(synCoordStr, 0, sizeof(synCoordStr));
        int x = syns[i]->GetX(), y = syns[i]->GetY();
        snprintf(synCoordStr, sizeof(synCoordStr), "(%d, %d)", x, y);
        float p = syns[i]->GetPerm();

        QLabel *synIdx = new QLabel(QString("%1: ").arg(i));
        QLabel *synCoordLab = new QLabel(synCoordStr);
        QLabel *synFiring = new QLabel(QString("%1").arg(syns[i]->IsFiring()? 1 : 0));
        QLabel *synPerm = new QLabel(QString("%1: ").arg(p));
        objGrid->addWidget(synIdx, 4+i, 0, 1, 1);
        objGrid->addWidget(synCoordLab, 4+i, 1, 1, 1);
        objGrid->addWidget(synFiring, 4+i, 2, 1, 1);
        objGrid->addWidget(synPerm, 4+i, 3, 1, 1);
    }

    // Note: must add the layout of widget before calling setWidget() or won't
    // be visible.
    objScroll->setWidget(scrollChild);
    objDetail->setLayout(objGroupLayout);
}

void QtUnit::contextMenuEvent(QContextMenuEvent *event)
{
    if (!this->IsClickable())
        return;
    QMenu menu(this);
    menu.addAction(showProximalConnections);
    menu.addAction(hideProximalConnections);
    menu.exec(event->globalPos());
}

