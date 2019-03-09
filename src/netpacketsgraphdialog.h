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

#ifndef NETPACKETSGRAPHDIALOG_H
#define NETPACKETSGRAPHDIALOG_H

#include "ui_netpacketsgraphdialog.h"

#include <QTimer>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QSvgGenerator>

#include "receivercore.h"
#include "settings.h"

class NetPacketsGraphDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(NetPacketsGraphDialog)

public:
    explicit NetPacketsGraphDialog(QWidget *parent = 0, ReceiverCore *receiverCore = 0);

    void startGraph();
    void stopGraph();
    void writeSettings();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    Ui::NetPacketsGraphDialogClass ui;

    QTimer *timerHour;

    quint8 time;

    quint16 dataMinute[61];
    quint16 dataHour[61];

    void clearData();

private slots:
    void setValue(quint16 packetsSpeed);

    void updateTimerHour();

    void onSave();
    void onRestoreDefaults();

    void onTimeChanged(int val);
    void toggledHLines(bool checked);
    void toggledVLines(bool checked);
    void toggledBackground(bool checked);
    void toggledFilled(bool checked);
    void toggledAntialiasing(bool checked);
    void toggledTooltip(bool checked);
};

#endif // NETPACKETSGRAPHDIALOG_H
