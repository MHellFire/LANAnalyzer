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

#include "netpacketsdialog.h"

NetPacketsDialog::NetPacketsDialog(QWidget *parent, ReceiverCore *receiverCore)
    : QDialog(parent)
{
    ui.setupUi(this);

    firstShow = true;

    connect(receiverCore, SIGNAL(signalNetPackets(quint64,quint64,quint64,quint64,quint64,quint64,quint64,quint64)), this, SLOT(setNetPackets(quint64,quint64,quint64,quint64,quint64,quint64,quint64,quint64)));
    
    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(close()));
}

void NetPacketsDialog::showEvent(QShowEvent *event)
{
    if (!firstShow)
    {
        resize(mySize);
        move(myPosition);
    }

    event->accept();
}

void NetPacketsDialog::closeEvent(QCloseEvent *event)
{
    firstShow = false;

    myPosition = pos();
    mySize = size();

    event->accept();
}

void NetPacketsDialog::setNetPackets(quint64 netTotal, quint64 netArp, quint64 netRarp, quint64 netIcmp, quint64 netIgmp, quint64 netUdp, quint64 netTcp, quint64 netOther)
{
    ui.labelNumTOTAL->setText(QString::number(netTotal));
    
    ui.labelNumARP->setText(QString::number(netArp));
    netArpReal = ((netArp * 100.0) / netTotal);
    ui.labelARP->setText(QString("%1%").arg(netArpReal, 0, 'f', 2));
    ui.progressBarARP->setValue(int(netArpReal));

    ui.labelNumRARP->setText(QString::number(netRarp));
    netRarpReal = ((netRarp * 100.0) / netTotal);
    ui.labelRARP->setText(QString("%1%").arg(netRarpReal, 0, 'f', 2));
    ui.progressBarRARP->setValue(int(netRarpReal));
    
    ui.labelNumICMP->setText(QString::number(netIcmp));
    netIcmpReal = ((netIcmp * 100.0) / netTotal);
    ui.labelICMP->setText(QString("%1%").arg(netIcmpReal, 0, 'f', 2));
    ui.progressBarICMP->setValue(int(netIcmpReal));
    
    ui.labelNumIGMP->setText(QString::number(netIgmp));
    netIgmpReal = ((netIgmp * 100.0) / netTotal);
    ui.labelIGMP->setText(QString("%1%").arg(netIgmpReal, 0, 'f', 2));
    ui.progressBarIGMP->setValue(int(netIgmpReal));
    
    ui.labelNumTCP->setText(QString::number(netTcp));
    netTcpReal = ((netTcp * 100.0) / netTotal);
    ui.labelTCP->setText(QString("%1%").arg(netTcpReal, 0, 'f', 2));
    ui.progressBarTCP->setValue(int(netTcpReal));
    
    ui.labelNumUDP->setText(QString::number(netUdp));
    netUdpReal = ((netUdp * 100.0) / netTotal);
    ui.labelUDP->setText(QString("%1%").arg(netUdpReal, 0, 'f', 2));
    ui.progressBarUDP->setValue(int(netUdpReal));

    ui.labelNumOTHER->setText(QString::number(netOther));
    netOtherReal = ((netOther * 100.0) / netTotal);
    ui.labelOTHER->setText(QString("%1%").arg(netOtherReal, 0, 'f', 2));
    ui.progressBarOTHER->setValue(int(netOtherReal));
}
