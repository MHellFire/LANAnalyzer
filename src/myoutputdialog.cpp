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

#include "myoutputdialog.h"

MyOutputDialog::MyOutputDialog(QWidget *parent, ReceiverCore *receiverCore)
    : QDialog(parent)
{
    ui.setupUi(this);

    connect(receiverCore, SIGNAL(signalMulticast(quint32,quint32,quint32,quint8)), this, SLOT(receivedMulticast(quint32,quint32,quint32,quint8)), Qt::QueuedConnection);

    connect(ui.treeWidgetMulticast, SIGNAL(itemSelectionChanged()), this, SLOT(onMulticastItemChanged()));

    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(close()));

    clear();

    firstShow = true;
}

void MyOutputDialog::showEvent(QShowEvent *event)
{
    if (!firstShow)
    {
        resize(mySize);
        move(myPosition);
    }

    event->accept();
}

void MyOutputDialog::closeEvent(QCloseEvent *event)
{
    firstShow = false;

    myPosition = pos();
    mySize = size();

    event->accept();
}

void MyOutputDialog::clear()
{
    disconnect(ui.treeWidgetMulticast, SIGNAL(itemSelectionChanged()), this, SLOT(onMulticastItemChanged()));

    ui.treeWidgetOther->clear();
    ui.treeWidgetMulticast->clear();

    mIP.clear();
    ips.clear();

    connect(ui.treeWidgetMulticast, SIGNAL(itemSelectionChanged()), this, SLOT(onMulticastItemChanged()));
}

QString MyOutputDialog::bytesToStr(quint64 bytes)
{
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes >= 1024 && bytes < 1048576) return QString("%1 KB").arg((bytes / 1024.0), 0, 'f', 2);
    if (bytes >= 1048576) return QString("%1 MB").arg((bytes / 1024.0 / 1024.0), 0, 'f', 2);

    return QString("");
}

void MyOutputDialog::onMulticastItemChanged()
{
    int item = ui.treeWidgetMulticast->indexOfTopLevelItem(ui.treeWidgetMulticast->currentItem());

    if (item > -1)
    {
        ui.treeWidgetOther->clear();

        for (int i = 0; i < ips.at(item).oIP.count(); ++i)
        {
            addr.S_un.S_addr = ips.at(item).oIP.at(i);
            ui.treeWidgetOther->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetOther, QStringList() << inet_ntoa(addr) << bytesToStr(ips.at(item).up.at(i)) << bytesToStr(ips.at(item).down.at(i))));
        }
    }
}

void MyOutputDialog::receivedMulticast(quint32 multicastIP, quint32 otherIP, quint32 length, quint8 direction)
{
    if (!mIP.contains(multicastIP))
    {
        mIP.append(multicastIP);

        IPs s;
        ips.append(s);

        addr.S_un.S_addr = multicastIP;
        ui.treeWidgetMulticast->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetMulticast, QStringList() << inet_ntoa(addr)));
    }

    int index = mIP.indexOf(multicastIP, 0);

    if (!ips.at(index).oIP.contains(otherIP))
    {
        ips[index].oIP.append(otherIP);
        ips[index].up.append(0);
        ips[index].down.append(0);

        if (ui.treeWidgetMulticast->indexOfTopLevelItem(ui.treeWidgetMulticast->currentItem()) == index)
        {
            addr.S_un.S_addr = otherIP;
            ui.treeWidgetOther->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetOther, QStringList() << inet_ntoa(addr) << bytesToStr(0) << bytesToStr(0)));
        }
    }

    int indexOther = ips.at(index).oIP.indexOf(otherIP, 0);

    if (direction == 0)
        ips[index].down[indexOther]+=length;
    else
        ips[index].up[indexOther]+=length;

    int item = ui.treeWidgetMulticast->indexOfTopLevelItem(ui.treeWidgetMulticast->currentItem());

    if ((item > -1) && (ui.treeWidgetOther->topLevelItemCount() != 0))
    {
        if (direction == 0)
            ui.treeWidgetOther->topLevelItem(indexOther)->setText(2, bytesToStr(ips.at(item).down.at(indexOther)));
        else
            ui.treeWidgetOther->topLevelItem(indexOther)->setText(1, bytesToStr(ips.at(item).up.at(indexOther)));
    }
}
