#include <stdio.h>

#include "qt.h"
#include "sensoryregion.h"
#include "sensoryinput.h"

QtSensoryRegion::QtSensoryRegion(QWidget *parent, SensoryRegion *patt)
    : QWidget(parent)
{
    this->parent = parent;
    this->pattern = patt;
    sensoryUnitGrid = NULL;
    motorUnitGrid = NULL;
}

QtSensoryRegion::~QtSensoryRegion()
{
}

QGridLayout* QtSensoryRegion::SensoryUnitGrid()
{
    if (sensoryUnitGrid)
        return sensoryUnitGrid;

    sensoryUnitGrid = new QGridLayout();
    sensoryUnitGrid->setSpacing(0);
    sensoryUnitGrid->setAlignment(Qt::AlignCenter);
    SensoryInput ***bits = pattern->GetInput();
    int w = pattern->GetWidth();
    int h = pattern->GetHeight();

    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            QtUnit *unit = new QtUnit(bits[i][j], NULL, NULL, 0, 3, 3);
            if (bits[i][j]->IsActive())
                unit->setBrushColor(ACTIVE_COLOR);
            sensoryUnitGrid->addWidget(unit, i, j, 1, 1);
        }
    }

    return sensoryUnitGrid;
}

QGridLayout* QtSensoryRegion::MotorUnitGrid()
{
    if (motorUnitGrid)
        return motorUnitGrid;

    motorUnitGrid = new QGridLayout();
    motorUnitGrid->setSpacing(0);
    motorUnitGrid->setAlignment(Qt::AlignCenter);
    SensoryRegion *motorPattern = pattern->GetMotorPattern();

    int w = motorPattern->GetWidth();
    int h = motorPattern->GetHeight();

    SensoryInput ***bits = motorPattern->GetInput();

    for (int i=0; i<h; i++) {
        for (int j=0; j<w; j++) {
            QtUnit *unit = new QtUnit(bits[i][j], NULL, NULL, 0, 6, 6);
            if (bits[i][j]->IsActive())
                unit->setBrushColor(ACTIVE_COLOR);
            motorUnitGrid->addWidget(unit, i, j, 1, 1);
        }
    }

    return motorUnitGrid;
}

SensoryRegion* QtSensoryRegion::GetPattern()
{
    return pattern;
}

