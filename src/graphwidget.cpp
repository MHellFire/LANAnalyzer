// Copyright © 2009 Mariusz Helfajer
//
// This file is part of LANAnalyzer.
//
// LANAnalyzer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LANAnalyzer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LANAnalyzer.  If not, see <http://www.gnu.org/licenses/>.

#include "graphwidget.h"

#include <math.h>

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
{
    hLines = true;
    vLines = false;
    background = true;
    filled = false;
    antialiasing = false;

    xLabel = "";
    yLabel = "";

    clearGraph();

    QFont myFont = font();
    myFont.setPixelSize(12);
    setFont(myFont);
    QFontMetrics fontMetrics(myFont);

    xLabelBoundingRect = fontMetrics.boundingRect(xLabel);
    yLabelBoundingRect = fontMetrics.boundingRect(yLabel);
}

void GraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!isActiveWindow())
    {
        event->accept();
        return;
    }

    QPoint widgetPosition = mapFromGlobal(event->globalPos());

    qreal x = (width()/8.0);
    qreal y = (height()/8.0);
    qreal w = (width()/8.0)*6.0;
    qreal h = (height()/8.0)*6.0;

    if ((widgetPosition.x() > x) && (widgetPosition.x() < x+w) && (widgetPosition.y() > y) && (widgetPosition.y() < y+h))
    {
        qreal d = (w/60.0), i = w+x;
        int j;

        QString text;

        for (j = 0; j < 61; ++j)
        {
            if ((widgetPosition.x() >= i-(d/2.0)) && (widgetPosition.x() <= i+(d/2.0)))
            {

                text = QString(tr("Time: %1\nPackets: %2").arg(QString::number(j)).arg(QString::number(data[j])));
                break;
            }

            i-=d;
        }

        QToolTip::showText(event->globalPos(), text, this);

        event->ignore();
    }

    event->accept();
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    paint(painter);
}

void GraphWidget::paint(QPainter &painter)
{
    if (antialiasing)
        painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::black);

    //
    qreal x = (width()/8.0);
    qreal y = (height()/8.0);
    qreal w = (width()/8.0)*6.0;
    qreal h = (height()/8.0)*6.0;

    if (background)
        painter.fillRect(QRectF(x, y, w, h), Qt::black);

    // x axis label
    painter.drawText(QPointF((width()/2.0) - (xLabelBoundingRect.width()/2.0), height() - (y/2.0 - xLabelBoundingRect.height()/2.0)), xLabel);

    // y axis label
    painter.save();
    painter.translate(((x/2.0) - yLabelBoundingRect.height()/2.0), ((height()/2.0) + yLabelBoundingRect.width()/2.0));
    painter.rotate(270);
    painter.drawText(0, 0, yLabel);
    painter.restore();

    // x axis values
    QFont myFont = font();
    myFont.setPixelSize(12);
    setFont(myFont);
    QFontMetrics fontMetrics(myFont);

    QString str;
    int n = 70;

    for (qreal i = x; i < w+x+x; i+=x)
    {
        str = QString::number(n-=10);
        QRect boundingRect = fontMetrics.boundingRect(str);
        boundingRect.setSize(boundingRect.size()*1.1);

        painter.drawText(QRectF(i-boundingRect.width()/2.0, h+y+(boundingRect.height()*0.2), boundingRect.width()*1.0, boundingRect.height()*1.0), Qt::AlignCenter, str);
    }

    // vertical grid lines
    if (vLines)
    {
        painter.setPen(Qt::darkGreen);

        for (qreal i = x; i < w+x+x; i+=x)
            painter.drawLine(QPointF(i, h+y), QPointF(i, y));

        painter.setPen(Qt::black);
    }

    // max value
    int max = 1;

    for (int i = 0; i < 61; ++i)
        if (data[i] >= max)
            max = data[i];

    // y axis values
    qreal dh;
    int c;

    if (max == 1) { dh = h; c = 1; goto end; }
    if (max == 2) { dh = h/2.0; c = 1; goto end; }
    if (max == 3) { dh = h/3.0; c = 1; goto end; }

    while (1)
    {
        if ((max%6) == 0) { dh = h/6.0; c = int(ceil(max/6.0)); break; }
        if ((max%5) == 0) { dh = h/5.0; c = int(ceil(max/5.0)); break; }
        if ((max%4) == 0) { dh = h/4.0; c = int(ceil(max/4.0)); break; }

        ++max;
    }

    end:

    n = 0;

    for (qreal i = y+h; i >= y-(dh/2.0); i-=dh)
    {
        str = QString::number(n);
        QRect boundingRect = fontMetrics.boundingRect(str);
        boundingRect.setSize(boundingRect.size()*1.1);

        painter.drawText(QRectF(x-boundingRect.width()-10.0, i-boundingRect.height()/2.0, boundingRect.width()*1.0, boundingRect.height()*1.0), Qt::AlignRight, str);

        n+=c;
    }

    // horizontal grid lines
    if (hLines)
    {
        painter.setPen(Qt::darkGreen);

        for (qreal i = y+h; i >= y-(dh/2.0); i-=dh)
            painter.drawLine(QPointF(x+1, i), QPointF(w+x-1, i));

        painter.setPen(Qt::black);
    }

    // graph

    int scale = n-c;

    painter.setPen(Qt::green);

    QPainterPath path;
    qreal d = (w/60.0), i = w+x, z;

    path.moveTo(w+x, y+h);

    for (int j = 0; j < 61 ; ++j)
    {
        z = y+h - qreal(h*data[j])/scale;
        path.lineTo(i, z);

        i-=d;
    }

    path.lineTo(i+d, h+y);
    path.lineTo(w+x,h+y);

    if (filled)
        painter.setBrush(QColor(0, 127,0,255));

    painter.drawPath(path);

    if (isEnabled())
        mouseMoveEvent(new QMouseEvent(QEvent::MouseMove, cursor().pos(), Qt::NoButton, QApplication::mouseButtons(), QApplication::keyboardModifiers()));
}

void GraphWidget::setData(quint16 data[])
{
    for (int i = 0; i < 61; ++i)
        this->data[i] = data[i];

    update();
}

void GraphWidget::clearGraph()
{
    for (int i = 0; i < 61; ++i)
        data[i] = 0;

    update();
}

void GraphWidget::setXLabel(const QString &label)
{
    xLabel = label;

    QFont myFont = font();
    myFont.setPixelSize(12);
    setFont(myFont);
    QFontMetrics fontMetrics(myFont);

    xLabelBoundingRect = fontMetrics.boundingRect(xLabel);

    update();
}

void GraphWidget::setYLabel(const QString &label)
{
    yLabel = label;

    QFont myFont = font();
    myFont.setPixelSize(12);
    setFont(myFont);
    QFontMetrics fontMetrics(myFont);

    yLabelBoundingRect = fontMetrics.boundingRect(yLabel);

    update();
}

void GraphWidget::setHLines(bool state)
{
    hLines = state;
    update();
}

void GraphWidget::setVLines(bool state)
{
    vLines = state;
    update();
}

void GraphWidget::setBackground(bool state)
{
    background = state;
    update();
}

void GraphWidget::setFilled(bool state)
{
    filled = state;
    update();
}

void GraphWidget::setAntialiasing(bool state)
{
    antialiasing = state;
    update();
}
