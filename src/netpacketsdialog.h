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

#ifndef NETPACKETSDIALOG_H
#define NETPACKETSDIALOG_H

#include "ui_netpacketsdialog.h"

#include <QCloseEvent>
#include <QShowEvent>

#include "receivercore.h"

class NetPacketsDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(NetPacketsDialog)

public:
    explicit NetPacketsDialog(QWidget *parent = 0, ReceiverCore *receiverCore = 0);

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    Ui::NetPacketsDialogClass ui;

    bool firstShow;

    QPoint myPosition;
    QSize mySize;

    qreal netArpReal,
          netRarpReal,
          netIcmpReal,
          netIgmpReal,
          netUdpReal,
          netTcpReal,
          netOtherReal;

public slots:
    void setNetPackets(quint64 netTotal, quint64 netArp, quint64 netRarp, quint64 netIcmp, quint64 netIgmp, quint64 netUdp, quint64 netTcp, quint64 netOther);
};

#endif // NETPACKETSDIALOG_H
