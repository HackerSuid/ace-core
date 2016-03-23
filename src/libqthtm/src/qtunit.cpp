#include <stdio.h>
#include <math.h>

#include "qt.h"
#include "column.h"
#include "dendritesegment.h"
#include "synapse.h"

QtUnit::QtUnit(GenericInput *GridNode, QGroupBox *objHtm, int c, int w, int h)
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

    cells = c;
    if (cells) {
        CellGrid = new QGridLayout();
        //CellGrid->setAlignment(Qt::AlignCenter);
        CellGrid->setSpacing(0);
        std::vector<Cell *> HtmCells = ((Column *)node)->GetCells();
        for (int i=0; i<sqrt(cells); i++) {
        //for (int i=0; i<cells; i++) {
            for (int j=0; j<sqrt(cells); j++) {
                QtCell *cell = new QtCell(HtmCells[/*i]);*/(i<<1)|(j<<0)]);
                CellGrid->addWidget(cell, i, /*0*/j);
            }
        }
        this->setLayout(CellGrid);
    }
}

QtUnit::~QtUnit()
{
}

void QtUnit::CreateActions()
{
    showConnections = new QAction("Show connections", this);
    connect(showConnections, SIGNAL(triggered()), this, SLOT(ShowConnections()));
    hideConnections = new QAction("Hide connections", this);
    connect(hideConnections, SIGNAL(triggered()), this, SLOT(HideConnections()));
}

void QtUnit::ShowConnections()
{
    _ToggleConnections(true);
}

void QtUnit::HideConnections()
{
    _ToggleConnections(false);
}

void QtUnit::_ToggleConnections(bool flag)
{
    if (toggled == flag)
        return;
    toggled = flag;
    Column *col = (Column *)node;
    DendriteSegment *segment = col->GetProximalDendriteSegment();
    std::vector<Synapse *> prox_syns = segment->GetSynapses();
    int rfs = col->GetRecFieldSz();
    //for (int i=0; i<rfs; i++)
    //    prox_syns[i]->ShowSynapse(flag);
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

    QLabel *overlapLab = new QLabel("overlap: ");
    QLabel *overlapVal = new QLabel(QString::number(((Column *)node)->GetOverlap()));
    objGrid->addWidget(overlapLab, 0, 0);
    objGrid->addWidget(overlapVal, 0, 1);

    QLabel *synapsesLab = new QLabel("synaptic details");
    objGrid->addWidget(synapsesLab, 1, 0, 1, 3);
    QLabel *idx = new QLabel("Idx");
    QLabel *coord = new QLabel("Coord");
    QLabel *firing = new QLabel("Firing");
    QLabel *perm = new QLabel("Perm");
    objGrid->addWidget(idx, 2, 0);
    objGrid->addWidget(coord, 2, 1);
    objGrid->addWidget(firing, 2, 2);
    objGrid->addWidget(perm, 2, 3);

    Column *col = (Column *)node;
    DendriteSegment *segment = col->GetProximalDendriteSegment();
    std::vector<Synapse *> syns  = segment->GetSynapses();
    for (int i=0; i<col->GetRecFieldSz(); i++) {
        char synCoordStr[32];
        memset(synCoordStr, 0, sizeof(synCoordStr));
        unsigned int x = syns[i]->GetX(), y = syns[i]->GetY();
        snprintf(synCoordStr, sizeof(synCoordStr), "(%d, %d)", x, y);
        float p = syns[i]->GetPerm();

        QLabel *synIdx = new QLabel(QString("%1: ").arg(i));
        QLabel *synCoordLab = new QLabel(synCoordStr);
        QLabel *synFiring = new QLabel(QString("%1").arg(syns[i]->IsFiring()? 1 : 0));
        QLabel *synPerm = new QLabel(QString("%1: ").arg(p));
        objGrid->addWidget(synIdx, 3+i, 0, 1, 1);
        objGrid->addWidget(synCoordLab, 3+i, 1, 1, 1);
        objGrid->addWidget(synFiring, 3+i, 2, 1, 1);
        objGrid->addWidget(synPerm, 3+i, 3, 1, 1);
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
    menu.addAction(showConnections);
    menu.addAction(hideConnections);
    menu.exec(event->globalPos());
}
