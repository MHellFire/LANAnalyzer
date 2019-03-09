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

#ifndef MYOUTPUTDIALOG_H
#define MYOUTPUTDIALOG_H

#include "ui_myoutputdialog.h"

#include <QCloseEvent>
#include <QShowEvent>

#include "receivercore.h"

class MyOutputDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(MyOutputDialog)

public:
    explicit MyOutputDialog(QWidget *parent = 0, ReceiverCore *receiverCore = 0);

    void clear();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    Ui::MyOutputDialogClass ui;

    bool firstShow;

    QPoint myPosition;
    QSize mySize;

    QList<quint32> mIP;

    struct IPs
    {
        QList<quint32> oIP;
        QList<quint64> up;
        QList<quint64> down;
    };
    QList<IPs> ips;

    struct in_addr addr;

    QString bytesToStr(quint64 bytes);

private slots:
    void onMulticastItemChanged();
    void receivedMulticast(quint32 multicastIP, quint32 otherIP, quint32 length, quint8 direction);
};

#endif // MYOUTPUTDIALOG_H
