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

#ifndef NETTRANSFERDIALOG_H
#define NETTRANSFERDIALOG_H

#include "ui_nettransferdialog.h"

#include <QCloseEvent>
#include <QShowEvent>

#include "receivercore.h"

class NetTransferDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(NetTransferDialog)

public:
    explicit NetTransferDialog(QWidget *parent = 0, ReceiverCore *receiverCore = 0);

    void setScale(const quint16 up, const quint16 down);

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    Ui::NetTransferDialogClass ui;

    bool firstShow;

    QPoint myPosition;
    QSize mySize;

    quint16 upScale, downScale;
    qreal upMax, downMax;

    QString bytesToStr(quint64 bytes);

private slots:
    void netTransfer(quint64 up, quint64 down);
    void netSpeed(const qreal &upSpeed, const qreal &downSpeed);
};

#endif // NETTRANSFERDIALOG_H
