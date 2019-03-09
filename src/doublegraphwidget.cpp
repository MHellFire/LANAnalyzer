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

#include "doublegraphwidget.h"

#include <math.h>

DoubleGraphWidget::DoubleGraphWidget(QWidget *parent)
    : QWidget(parent)
{
    hLines = true;
    vLines = false;
    background = true;
    filled = false;
    antialiasing = false;
    up = true;
    down = true;

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

void DoubleGraphWidget::mouseMoveEvent(QMouseEvent *event)
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
                text = QString(tr("Time: %1").arg(QString::number(j)));

                if (up)
                    text.append(tr("\nUpload: %1").arg(QString::number(dataUp[j])));

                if (down)
                    text.append(tr("\nDownload: %1").arg(QString::number(dataDown[j])));

                break;
            }

            i-=d;
        }

        QToolTip::showText(event->globalPos(), text, this);

        event->ignore();
    }

    event->accept();
}

void DoubleGraphWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    paint(painter);
}

void DoubleGraphWidget::paint(QPainter &painter)
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

    if (down)
    {
        for (int i = 0; i < 61; ++i)
            if (dataDown[i] >= max)
                max = dataDown[i];
    }

    if (up)
    {
        for (int i = 0; i < 61; ++i)
            if (dataUp[i] >= max)
                max = dataUp[i];
    }

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

    if (down)
    {
        painter.setPen(Qt::green);

        QPainterPath pathDown;
        qreal d =(w/60.0), i = w+x, z;

        pathDown.moveTo(w+x, y+h);

        for (int j = 0; j < 61; ++j)
        {
            z = y+h - qreal(h*dataDown[j])/scale;
            pathDown.lineTo(i, z);

            i-=d;
        }

        pathDown.lineTo(i+d, h+y);
        pathDown.lineTo(w+x,h+y);

        if (filled)
            painter.setBrush(QColor(0, 127,0,255));

        painter.drawPath(pathDown);
    }

    if (up)
    {
        painter.setPen(Qt::red);

        QPainterPath pathUp;

        qreal d = (w/60.0), i = w+x, z;

        pathUp.moveTo(w+x, y+h);

        for (int j = 0; j < 61; ++j)
        {
            z = y+h - qreal(h*dataUp[j])/scale;
            pathUp.lineTo(i, z);

            i-=d;
        }

        pathUp.lineTo(i+d, h+y);
        pathUp.lineTo(w+x,h+y);

        if (filled)
            painter.setBrush(QColor(127, 0 ,0 ,255));

        painter.drawPath(pathUp);
    }

    if (isEnabled())
        mouseMoveEvent(new QMouseEvent(QEvent::MouseMove, cursor().pos(), Qt::NoButton, QApplication::mouseButtons(), QApplication::keyboardModifiers()));
}

void DoubleGraphWidget::setData(const quint16 dataUp[], const quint16 dataDown[])
{
    for (int i = 0; i < 61; ++i)
    {
        this->dataUp[i] = dataUp[i];
        this->dataDown[i] = dataDown[i];
    }

    update();
}

void DoubleGraphWidget::clearGraph()
{
    for (int i = 0; i < 61; ++i)
    {
        dataUp[i] = 0;
        dataDown[i] = 0;
    }

    update();
}

void DoubleGraphWidget::setXLabel(const QString &label)
{
    xLabel = label;

    QFont myFont = font();
    myFont.setPixelSize(12);
    setFont(myFont);
    QFontMetrics fontMetrics(myFont);

    xLabelBoundingRect = fontMetrics.boundingRect(xLabel);

    update();
}

void DoubleGraphWidget::setYLabel(const QString &label)
{
    yLabel = label;

    QFont myFont = font();
    myFont.setPixelSize(12);
    setFont(myFont);
    QFontMetrics fontMetrics(myFont);

    yLabelBoundingRect = fontMetrics.boundingRect(yLabel);

    update();
}

void DoubleGraphWidget::setHLines(bool state)
{
    hLines = state;
    update();
}

void DoubleGraphWidget::setVLines(bool state)
{
    vLines = state;
    update();
}

void DoubleGraphWidget::setBackground(bool state)
{
    background = state;
    update();
}

void DoubleGraphWidget::setFilled(bool state)
{
    filled = state;
    update();
}

void DoubleGraphWidget::setAntialiasing(bool state)
{
    antialiasing = state;
    update();
}

void DoubleGraphWidget::setUp(bool state)
{
    up = state;
    update();
}

void DoubleGraphWidget::setDown(bool state)
{
    down = state;
    update();
}
