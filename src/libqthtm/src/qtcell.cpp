#include <stdio.h>

#include "qt.h"

QtCell::QtCell(Cell *cell, int w, int h)
{
    // don't resize on repaint.
    setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    brushColor = INACTIVE_COLOR;

    sizeW = w;
    sizeH = h;

    this->cell = cell;
}

QtCell::~QtCell()
{
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
    if (cell->IsActive())
        p.setBrush(ACTIVE_COLOR);
    else if (cell->IsPredicted())
        p.setBrush(PREDICTED_COLOR);
    else
        p.setBrush(INACTIVE_COLOR);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap));
    p.drawRoundedRect(r, 15.0, 15.0);
}

void QtCell::mousePressEvent(QMouseEvent *event)
{
    printf("[0x%08x] a=%d p=%d l=%d\n", cell, cell->IsActive(), cell->IsPredicted(), cell->IsLearning());
}

