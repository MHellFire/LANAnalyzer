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

#ifndef DEVICESDIALOG_H
#define DEVICESDIALOG_H

#define _WIN32_WINNT 0x0501 // ip6tos

#include "ui_devicesdialog.h"

#include <QMessageBox>

//#include <winsock2.h>
#include <ws2tcpip.h>

#include "WpdPack/Include/pcap.h"

#include "settings.h"

class DevicesDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(DevicesDialog)

public:
    explicit DevicesDialog(QWidget *parent = 0);
    ~DevicesDialog();

    bool obtainAdapters(const QString &deviceName);
    bool execute();

    pcap_if_t *device;

private:
    Ui::DevicesDialogClass ui;

    QStringList name,
                description,
                loopback,
                family,
                address,
                netmask,
                broadaddr,
                dstaddr,
                mac,
                type,
                typeDesc;
    QList<int> addrList;
    
    pcap_if_t *allDevices;

    bool showIPv4, showIPv6;

    void displayLabels(quint16 deviceIndex);
    char* iptos(u_long in);
    char* ip6tos(struct sockaddr *sockaddr, char *address, int addrlen);

private slots:
    void onOK();
    void onDeviceChanged();
    void onDeviceAddressChanged(int index);

signals:
    void infoMessage(quint8 type, const QString &title, const QString &message);
};

#endif // DEVICESDIALOG_H
