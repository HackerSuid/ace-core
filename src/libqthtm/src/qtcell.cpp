#include <stdio.h>

#include "qt.h"
#include "column.h"
#include "cell.h"
#include "dendritesegment.h"
#include "synapse.h"

QtCell::QtCell(
    QWidget *parent,
    Cell *cell,
    QGridLayout *htmGrid,
    QGridLayout *sensoryGrid,
    QGridLayout *motorGrid,
    QGridLayout *locGrid,
    int w, int h
) : QWidget(parent)
{
    /* don't resize on repaint. */
    setAttribute(Qt::WA_StaticContents);
    /* implement hover-over detection */
    //setMouseTracking(true);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    brushColor = INACTIVE_COLOR;

    sizeW = w;
    sizeH = h;

    toggled = false;

    this->cell = cell;
/*
    if (cell->IsActive())
        setBrushColor(ACTIVE_COLOR);
    else if (cell->IsPredicted())
        setBrushColor(PREDICTED_COLOR);
    else
        setBrushColor(INACTIVE_COLOR);
*/

    this->htmGrid = htmGrid;
    this->sensoryGrid = sensoryGrid;
    this->motorGrid = motorGrid;
    this->locGrid = locGrid;

    CreateActions();
}

QtCell::~QtCell()
{
}

void QtCell::CreateActions()
{
    showDistalConnections =
        new QAction("Show distal connections", this);
    connect(
        showDistalConnections,
        SIGNAL(triggered()),
        this,
        SLOT(ShowDistalConnections())
    );
    hideDistalConnections =
        new QAction("Hide distal connections", this);
    connect(
        hideDistalConnections,
        SIGNAL(triggered()),
        this,
        SLOT(HideDistalConnections())
    );
}

void QtCell::ShowDistalConnections()
{
    _ToggleDistalConnections(true);
}

void QtCell::HideDistalConnections()
{
    _ToggleDistalConnections(false);
}

void QtCell::_ToggleDistalConnections(bool flag)
{
    if (toggled == flag)
        return;
    toggled = flag;

    printf("Toggling distal synapses\n");
    std::vector<DendriteSegment *> segments = cell->GetSegments();
    printf("Cell has %u segments.\n", segments.size());

    for (unsigned int seg=0; seg<segments.size(); seg++) {
        std::vector<Synapse *> distalSyns =
            segments[seg]->GetSynapses();
        printf("\tseg %u has %u synapses\n", seg, distalSyns.size());
        unsigned int numActiveLat=0, numActiveMot=0;
        unsigned int numActiveLoc=0, numActiveSen=0;
        for (unsigned int syn=0; syn<distalSyns.size(); syn++) {
            QGridLayout *srcLayout = NULL;
            if (distalSyns[syn]->IsLateral()) {
                if (distalSyns[syn]->IsFiring())
                    numActiveLat++;
                srcLayout = htmGrid;

                QtUnit *colWidget = (QtUnit *)srcLayout->itemAtPosition(
                    distalSyns[syn]->GetY(),
                    distalSyns[syn]->GetX()
                )->widget();

                Cell *srcCell =
                    (Cell *)distalSyns[syn]->GetSource();
                QList<QtCell *> childCells = 
                    colWidget->findChildren<QtCell *>();
                QtCell *srcWidget =
                    childCells.at(srcCell->GetColIdx());

                if (flag) {
                    srcWidget->SaveBrushColor();
                    srcWidget->setBrushColor(
                        srcWidget->IsActive() ? HIGHLIGHT_ACTIVE :
                                                HIGHLIGHT_INACTIVE
                    );
                } else
                    srcWidget->RestoreBrushColor();
                srcWidget->repaint();
            } else {
                if (distalSyns[syn]->IsMotor()) {
                    if (distalSyns[syn]->IsFiring()) {
                        printf("[mot] (%d, %d) %d\n",
                            distalSyns[syn]->GetX(),
                            distalSyns[syn]->GetY(),
                            distalSyns[syn]->IsFiring());
                        numActiveMot++;
                    }
                    srcLayout = motorGrid;
                }
                if (distalSyns[syn]->IsLocation()) {
                    if (distalSyns[syn]->IsFiring()) {
                        printf("[loc] (%d, %d) %d\n",
                            distalSyns[syn]->GetX(),
                            distalSyns[syn]->GetY(),
                            distalSyns[syn]->IsFiring());
                        numActiveLoc++;
                    }
                    srcLayout = locGrid;
                }
                if (distalSyns[syn]->IsSensory()) {
                    if (distalSyns[syn]->IsFiring()) {
                        printf("[sen] (%d, %d) %d\n",
                            distalSyns[syn]->GetX(),
                            distalSyns[syn]->GetY(),
                            distalSyns[syn]->IsFiring());
                        numActiveSen++;
                    }
                    srcLayout = htmGrid;
                }

                printf("getting source qtunit\n");
                QtUnit *srcWidget = (QtUnit *)srcLayout->itemAtPosition(
                    distalSyns[syn]->GetY(),
                    distalSyns[syn]->GetX()
                )->widget();

                if (flag) {
                    srcWidget->SaveBrushColor();
                    srcWidget->setBrushColor(
                        srcWidget->IsActive() ? HIGHLIGHT_ACTIVE :
                                                HIGHLIGHT_INACTIVE
                    );
                } else
                    srcWidget->RestoreBrushColor();
                srcWidget->repaint();
            }

            //printf("\t[%d] (%d, %d)\n",
                //i, prox_syns[i]->GetX(), prox_syns[i]->GetY());
        }
        printf("\t\t%u total: %u sen %u mot %u loc %u "
               "lat\n",
            distalSyns.size(), numActiveSen,
            numActiveMot, numActiveLoc, numActiveLat
        );
    }
}

QSize QtCell::sizeHint() const
{
    QSize sz(sizeW, sizeH);
    return sz;
}

void QtCell::setBrushColor(const QColor &newColor)
{
    brushColor = newColor;
}

QColor QtCell::getBrushColor() const
{
    return brushColor;
}

void QtCell::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QRect r(0, 0, sizeHint().width(), sizeHint().height());

/*
    if (cell->IsActive())
        p.setBrush(ACTIVE_COLOR);
    else if (cell->IsPredicted()) {
        p.setBrush(PREDICTED_COLOR);
    }
    else
        p.setBrush(INACTIVE_COLOR);
*/

    p.setBrush(brushColor);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap));
    p.drawRoundedRect(r, 15.0, 15.0);
}

void QtCell::mousePressEvent(QMouseEvent *event)
{
    printf("[0x%08x] a=%d p=%d l=%d\n",
        cell, cell->IsActive(), cell->IsPredicted(),
        cell->IsLearning()
    );
/*
    DendriteSegment *seg = cell->GetMostActiveSegment();
    if (seg) {
        std::vector<Synapse *> syns = seg->GetSynapses();
        for (int i=0; i<seg->GetNumSynapses(); i++)
            printf("(%d, %d): %d %f\n",
                syns[i]->GetX(),
                syns[i]->GetY(),
                syns[i]->GetSource()->IsActive(),
                syns[i]->GetPerm()
            );
    }
*/
}

void QtCell::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(showDistalConnections);
    menu.addAction(hideDistalConnections);
    menu.exec(event->globalPos());
}

void QtCell::enterEvent(QEvent *event)
{
    SaveBrushColor();
    setBrushColor(HOVER_COLOR);
    repaint();
}

void QtCell::leaveEvent(QEvent *event)
{
    RestoreBrushColor();
    repaint();
}

