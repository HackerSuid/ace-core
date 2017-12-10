#include <stdio.h>
#include <numeric>

#include "qt.h"
#include "htm.h"
#include "htmsublayer.h"
#include "pooling_layer.h"
#include "column.h"

QtHtm::QtHtm(QWidget *parent, Htm *htm)
    : QWidget(parent)
{
    this->htm = htm;
    htmUnitGrid = NULL;
    poolUnitGrid = NULL;
}

QtHtm::~QtHtm()
{
}

/*
 * Notice: only one sublayer is allowed by the code.
 * TODO: allow multiple sublayers and regions in a hierarchy to be
 * configured.
 */
QGridLayout* QtHtm::UnitGrid(QGroupBox *objHtm)
{
    if (htmUnitGrid)
        return htmUnitGrid;

    htmUnitGrid = new QGridLayout();
    htmUnitGrid->setSpacing(0);
    htmUnitGrid->setAlignment(Qt::AlignCenter);
    HtmSublayer **sublayers = htm->GetSublayers();
    int w = sublayers[0]->GetWidth();
    int h = sublayers[0]->GetHeight();
    int c = sublayers[0]->GetDepth();
    Column ***cols = sublayers[0]->GetInput();

    QtSensoryRegion *qtinput =
        ((QtFront *)parentWidget())->GetCurrentInputDisplay();
    QGridLayout *sensoryGrid = qtinput->SensoryUnitGrid();
    QGridLayout *motorGrid = qtinput->MotorUnitGrid();
    QGridLayout *locGrid = qtinput->LocationUnitGrid();
    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            QtUnit *unit = new QtUnit(
                cols[i][j],
                objHtm,
                htmUnitGrid,
                sensoryGrid,
                motorGrid,
                locGrid,
                c
            );
            unit->SetClickable(true);
            if (cols[i][j]->IsActive())
                unit->setBrushColor(ACTIVE_COLOR);
            htmUnitGrid->addWidget(unit, i, j, 1, 1);
        }
    }

    return htmUnitGrid;
}

QGridLayout* QtHtm::PoolUnitGrid()
{
    if (poolUnitGrid)
        return poolUnitGrid;

    int h = htm->GetPoolingLayer()->GetHeight();
    int w = htm->GetPoolingLayer()->GetWidth();

    poolUnitGrid = new QGridLayout();
    poolUnitGrid->setSpacing(0);
    poolUnitGrid->setAlignment(Qt::AlignCenter);
    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            QtUnit *unit = new QtUnit(
                NULL,
                NULL,
                htmUnitGrid,
                NULL,
                NULL,
                NULL,
                0
            );
            unit->SetClickable(true);
            //if (cols[i][j]->IsActive())
            //    unit->setBrushColor(ACTIVE_COLOR);
            poolUnitGrid->addWidget(unit, i, j, 1, 1);
        }
    }

    return poolUnitGrid;
}

float QtHtm::SlidingWindowAvg(std::list<float> window)
{
    if (window.size() == 0)
        return 0.0;
    return std::accumulate(
        window.begin(),
        window.end(),
        0.0
    ) / window.size();
}

float QtHtm::PredictionComprehensionMetric()
{
    HtmSublayer *sublayer = (htm->GetSublayers())[0];

    return SlidingWindowAvg(
        sublayer->GetPredictionComprehensionWindow()
    );
}

float QtHtm::PredictionSpecificityMetric()
{
    HtmSublayer *sublayer = (htm->GetSublayers())[0];

    return SlidingWindowAvg(
        sublayer->GetPredictionSpecificityWindow()
    );
}

/*
 * As above, only one region is assumed at the moment.
 *
 * Associating the dendrite segments with QtUnit inputs this way instead of
 * when constructing a QtUnit itself avoids adding special-case code to
 * QtUnit(). QtUnits in the QtSensoryRegion have no purpose for it. Columns
 * on the other hand will always require this association of data structures.
 */
void QtHtm::SetQtSynapses(QGridLayout *inputGrid)
{
    HtmSublayer **sublayers = htm->GetSublayers();
    Column ***cols = sublayers[0]->GetInput();
    int w = sublayers[0]->GetWidth();
    int h = sublayers[0]->GetHeight();

/*
    for (int i=0; i<h; i++)
        for (int j=0; j<w; j++)
            cols[i][j]->AssociateQtInput(inputGrid);
 */
}

