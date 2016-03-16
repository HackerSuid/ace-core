#include <stdio.h>

#include "qt.h"
#include "htm.h"
#include "htmregion.h"
#include "column.h"

QtHtm::QtHtm(QWidget *parent, Htm *htm)
    : QWidget(parent)
{
    this->htm = htm;
    unitGrid = NULL;
}

QtHtm::~QtHtm()
{
}

/*
 * Notice: only one region is allowed by the code.
 * TODO: allow multiple regions in a hierarchy to be configured.
 */
QGridLayout* QtHtm::UnitGrid(QGroupBox *objHtm)
{
    if (unitGrid)
        return unitGrid;

    unitGrid = new QGridLayout();
    unitGrid->setSpacing(0);
    unitGrid->setAlignment(Qt::AlignCenter);
    HtmRegion **regions = htm->GetRegions();
    int w = regions[0]->GetWidth();
    int h = regions[0]->GetHeight();
    int c = regions[0]->GetDepth();
    Column ***cols = regions[0]->GetInput();

    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            QtUnit *unit = new QtUnit(cols[i][j], objHtm, c);
            unit->SetClickable(true);
            if (cols[i][j]->IsActive())
                unit->setBrushColor(ACTIVE_COLOR);
            unitGrid->addWidget(unit, i, j, 1, 1);
        }
    }

    return unitGrid;
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
    HtmRegion **regions = htm->GetRegions();
    Column ***cols = regions[0]->GetInput();
    int w = regions[0]->GetWidth();
    int h = regions[0]->GetHeight();

/*
    for (int i=0; i<h; i++)
        for (int j=0; j<w; j++)
            cols[i][j]->AssociateQtInput(inputGrid);
 */
}

