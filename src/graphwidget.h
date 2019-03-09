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

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QToolTip>
#include <QApplication>
#include <QPainter>

class GraphWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(GraphWidget)

public:
    explicit GraphWidget(QWidget *parent = 0);

    void paint(QPainter &painter);

    void setXLabel(const QString &label);
    void setYLabel(const QString &label);
    void setHLines(bool state);
    void setVLines(bool state);
    void setBackground(bool state);
    void setFilled(bool state);
    void setAntialiasing(bool state);

    void clearGraph();

    void setData(quint16 data[]);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private:
    QRect xLabelBoundingRect, yLabelBoundingRect, xValBoundingRect, yValBoundingRect;

    quint16 data[61];

    bool hLines, vLines, background, filled, antialiasing;
    QString xLabel, yLabel;
};

#endif // GRAPHWIDGET_H
