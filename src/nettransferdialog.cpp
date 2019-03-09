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

#include "nettransferdialog.h"

NetTransferDialog::NetTransferDialog(QWidget *parent, ReceiverCore *receiverCore)
    : QDialog(parent)
{
    ui.setupUi(this);

    upScale = 0;
    downScale = 0;
    upMax = 0.0;
    downMax = 0.0;

    firstShow = true;

    connect(receiverCore, SIGNAL(signalNetTransfer(quint64,quint64)), this, SLOT(netTransfer(quint64,quint64)));
    connect(receiverCore, SIGNAL(signalNetSpeed(qreal,qreal)), this, SLOT(netSpeed(qreal,qreal)));

    connect(ui.pushButtonOk, SIGNAL(clicked()), this, SLOT(close()));
}

void NetTransferDialog::showEvent(QShowEvent *event)
{
    if (!firstShow)
    {
        resize(mySize);
        move(myPosition);
    }

    event->accept();
}

void NetTransferDialog::closeEvent(QCloseEvent *event)
{
    firstShow = false;

    myPosition = pos();
    mySize = size();

    event->accept();
}

void NetTransferDialog::setScale(const quint16 up, const quint16 down)
{
    upMax = 0.0;
    downMax = 0.0;
    upScale = up;
    downScale = down;

    // 0 = auto scaling

    if (up != 0)
        ui.labelMaxUp->setText(QString("%1 KB/s").arg(up));

    if (down != 0)
        ui.labelMaxDown->setText(QString("%1 KB/s").arg(down));
}

QString NetTransferDialog::bytesToStr(quint64 bytes)
{
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes >= 1024 && bytes < 1048576) return QString("%1 KB").arg((bytes / 1024.0), 0, 'f', 2);
    if (bytes >= 1048576) return QString("%1 MB").arg((bytes / 1024.0 / 1024.0), 0, 'f', 2);

    return QString("");
}

void NetTransferDialog::netTransfer(quint64 up, quint64 down)
{
    ui.labelUp->setText(bytesToStr(up));
    ui.labelDown->setText(bytesToStr(down));
}

void NetTransferDialog::netSpeed(const qreal &upSpeed, const qreal &downSpeed)
{
    ui.labelUpSpeed->setText(QString("%1 KB/s").arg(upSpeed, 0, 'f', 2));
    ui.labelDownSpeed->setText(QString("%1 KB/s").arg(downSpeed, 0, 'f', 2));

    if (upScale == 0) // auto scaling
    {
        if (upSpeed > upMax)
            upMax = upSpeed;

        ui.labelMaxUp->setText(QString("%1 KB/s").arg(upMax, 0, 'f', 2));
        ui.progressBarUp->setValue(int(upSpeed * 100.0 / upMax));
    }
    else
    {
        ui.progressBarUp->setValue(int(upSpeed * 100.0 / upScale));
    }

    if (downScale == 0) // auto scaling
    {
        if (downSpeed > downMax)
            downMax = downSpeed;

        ui.labelMaxDown->setText(QString("%1 KB/s").arg(downMax, 0, 'f', 2));
        ui.progressBarDown->setValue(int(downSpeed * 100.0 / downMax));
    }
    else
    {
        ui.progressBarDown->setValue(int(downSpeed * 100.0 / downScale));
    }
}
