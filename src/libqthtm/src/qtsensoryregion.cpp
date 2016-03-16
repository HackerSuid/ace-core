#include <stdio.h>

#include "qt.h"
#include "sensoryregion.h"
#include "sensoryinput.h"

QtSensoryRegion::QtSensoryRegion(QWidget *parent, SensoryRegion *patt)
    : QWidget(parent)
{
    this->parent = parent;
    this->pattern = patt;
    unitGrid = NULL;
}

QtSensoryRegion::~QtSensoryRegion()
{
}

QGridLayout* QtSensoryRegion::UnitGrid()
{
    if (unitGrid)
        return unitGrid;

    unitGrid = new QGridLayout();
    unitGrid->setSpacing(0);
    unitGrid->setAlignment(Qt::AlignCenter);
    SensoryInput ***bits = pattern->GetInput();
    int w = pattern->GetWidth();
    int h = pattern->GetHeight();

    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            QtUnit *unit = new QtUnit(bits[i][j], NULL, 0, 8, 8);
            if (bits[i][j]->IsActive())
                unit->setBrushColor(ACTIVE_COLOR);
            unitGrid->addWidget(unit, i, j, 1, 1);
        }
    }

    return unitGrid;
}

SensoryRegion* QtSensoryRegion::GetPattern()
{
    return pattern;
}

