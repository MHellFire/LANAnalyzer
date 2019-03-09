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

#include "devicesdialog.h"

DevicesDialog::DevicesDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    showIPv4 = Settings::devicesDialog.showIPv4;
    showIPv6 = Settings::devicesDialog.showIPv6;

    connect(ui.treeWidgetDevices, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onOK()));

    connect(ui.pushButtonInfo, SIGNAL(toggled(bool)), ui.widgetInfo, SLOT(setVisible(bool)));
    ui.widgetInfo->hide();
    ui.pushButtonInfo->setChecked(Settings::devicesDialog.showDevicesInfo);

    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));
}

DevicesDialog::~DevicesDialog()
{
    // we don't need any more the device list, free it
    //pcap_freealldevs(allDevices);
}

void DevicesDialog::displayLabels(quint16 deviceIndex)
{
    ui.comboBoxAddresses->clear();

    ui.labelLoopback->setText(loopback.at(deviceIndex));
    ui.labelType->setText(type.at(deviceIndex));
    ui.labelTypeDescription->setText(typeDesc.at(deviceIndex));

    int j;

    for (j = 0; j < addrList.size(); ++j)
    {
        if (addrList.at(j) == deviceIndex)
        {
            if ((family.at(j) == "AF_INET" && !showIPv4) || (family.at(j) == "AF_INET6" && !showIPv6))
                continue;
            else
                ui.comboBoxAddresses->addItem(address.at(j));
        }
    }

    for (j = 0; j < addrList.size(); ++j)
    {
        if (addrList.at(j) == deviceIndex)
        {
            if ((family.at(j) == "AF_INET" && !showIPv4) || (family.at(j) == "AF_INET6" && !showIPv6))
                continue;
            else
                break;
        }
    }

    if (j != addrList.size())
    {
        ui.labelAddressFamily->setText(family.at(j));
        ui.labelMask->setText(netmask.at(j));
        ui.labelBroadcast->setText(broadaddr.at(j));
        ui.labelGateway->setText(dstaddr.at(j));
    }
    else
    {
        ui.labelAddressFamily->setText("");
        ui.labelMask->setText("");
        ui.labelBroadcast->setText("");
        ui.labelGateway->setText("");
    }

    // set first address as default
    ui.comboBoxAddresses->setCurrentIndex(0);
}

// from tcptraceroute, convert a numeric IP address to a string
char* DevicesDialog::iptos(u_long in)
{
    //#define IPTOSBUFFERS 12
    static char output[12][3*4+3+1];
    static short which;
    u_char *p;

    p = (u_char *)&in;
    which = (which + 1 == 12 ? 0 : which + 1);
    sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

    return output[which];
}

// ws2tcpip.h
char* DevicesDialog::ip6tos(struct sockaddr *sockaddr, char *address, int addrlen)
{
    socklen_t sockaddrlen;

    //#ifdef WIN32

    sockaddrlen = sizeof(struct sockaddr_in6);

    //#else
    //sockaddrlen = sizeof(struct sockaddr_storage);
    //#endif

    if (getnameinfo(sockaddr, sockaddrlen, address, addrlen, NULL, 0, NI_NUMERICHOST) != 0)
        address = NULL;

    return address;
}

bool DevicesDialog::obtainAdapters(const QString &deviceName)
{
    char errbuf[PCAP_ERRBUF_SIZE];

    // retrieve the device list from the local machine
    if (pcap_findalldevs(&allDevices, errbuf) == -1)
    {
        // 3 - critical
        emit infoMessage(3, tr("Retrieving the device list"), tr("Error in pcap_findalldevs: \"%1\"").arg(QString(errbuf)));

        return false;
    }

    pcap_addr_t *a;
    char ip6str[128];
    pcap_t *adhandle;

    int i = 0;

    // print the list
    for (device = allDevices; device != NULL; device = device->next)
    {
        // name
        name.append(device->name);

        // description
        if (device->description)
            description.append(device->description);
        else
            description.append(tr("not available"));

        // loopback address
        loopback.append((device->flags & PCAP_IF_LOOPBACK) ? tr("yes") : tr("no"));

        // IP addresses
        for (a = device->addresses; a; a = a->next)
        {
            addrList.append(i);
            switch(a->addr->sa_family)
            {
                case AF_INET:
                    family.append(tr("AF_INET"));

                    if (a->addr)
                        address.append(iptos(((struct sockaddr_in*)a->addr)->sin_addr.s_addr));
                    else
                        address.append(tr("n/a"));

                    if (a->netmask)
                        netmask.append(iptos(((struct sockaddr_in*)a->netmask)->sin_addr.s_addr));
                    else
                        netmask.append(tr("n/a"));

                    if (a->broadaddr)
                        broadaddr.append(iptos(((struct sockaddr_in*)a->broadaddr)->sin_addr.s_addr));
                    else
                        broadaddr.append(tr("n/a"));

                    if (a->dstaddr)
                        dstaddr.append(iptos(((struct sockaddr_in*)a->dstaddr)->sin_addr.s_addr));
                    else
                        dstaddr.append(tr("n/a"));

                    break;

                case AF_INET6:
                    family.append(tr("AF_INET6"));

                    if (a->addr)
                        address.append(ip6tos(a->addr, ip6str, sizeof(ip6str)));
                    else
                        address.append(tr("n/a"));

                    netmask.append(tr("n/a"));
                    broadaddr.append(tr("n/a"));
                    dstaddr.append(tr("n/a"));

                    break;

                default:
                    family.append(QString::number(a->addr->sa_family));
                    address.append(tr("unknown"));
                    netmask.append(tr("unknown"));
                    broadaddr.append(tr("unknown"));
                    dstaddr.append(tr("unknown"));

                    break;
                }
        }

        // open the device
        if ((adhandle = pcap_open_live(device->name, 65536, 1, 1000, errbuf)) == NULL)
        {
            type.append(tr("n/a"));
            typeDesc.append(tr("n/a"));
        }
        else
        {
            type.append(pcap_datalink_val_to_name(pcap_datalink(adhandle)));
            typeDesc.append(pcap_datalink_val_to_description(pcap_datalink(adhandle)));
        }

        ++i;
    }

    if (i == 0)
    {
        // 3 - critical
        emit infoMessage(3, tr("Retrieving the device list"), tr("No interfaces found! Make sure WinPcap is installed."));

        return false;
    }

    int j;

    // show devices
    for (j = 0; j < i; ++j)
    {
        ui.treeWidgetDevices->addTopLevelItem(new QTreeWidgetItem(QStringList() << name.at(j) << description.at(j)));
        ui.treeWidgetDevices->topLevelItem(j)->setIcon(0, QIcon(":/images/16_device.png"));
    }

    // restore previouse device
    for (j = 0; j < i; ++j)
    {
        if (name.at(j) == deviceName)
        {
            ui.treeWidgetDevices->setCurrentItem(ui.treeWidgetDevices->topLevelItem(j));
            displayLabels(j);
            break;
        }
    }

    // if device not found or first application run then set first device as default
    if (j == i)
    {
        ui.treeWidgetDevices->setCurrentItem(ui.treeWidgetDevices->topLevelItem(0));
        displayLabels(0);
    }

    ui.treeWidgetDevices->resizeColumnToContents(0);
    ui.treeWidgetDevices->resizeColumnToContents(1);

    connect(ui.treeWidgetDevices, SIGNAL(itemSelectionChanged()), this, SLOT(onDeviceChanged()));
    connect(ui.comboBoxAddresses, SIGNAL(activated(int)), this, SLOT(onDeviceAddressChanged(int)));

    return true;
}

void DevicesDialog::onDeviceAddressChanged(int index)
{
    int i;

    // search first occur of current device index at addrList, look at obtainAdapters() function
    // addrList example: 0,0,0,0,1,1,2,2,3,3,3,3, where 0 is first device, 1 the second one...
    for (i = 0; i < addrList.size(); ++i)
        if (addrList.at(i) == (ui.treeWidgetDevices->indexOfTopLevelItem(ui.treeWidgetDevices->currentItem())))
            break;

    ui.labelAddressFamily->setText(family.at(i+index));
    ui.labelMask->setText(netmask.at(i+index));
    ui.labelBroadcast->setText(broadaddr.at(i+index));
    ui.labelGateway->setText(dstaddr.at(i+index));
}

void DevicesDialog::onDeviceChanged()
{
    displayLabels(ui.treeWidgetDevices->indexOfTopLevelItem(ui.treeWidgetDevices->currentItem()));
}

bool DevicesDialog::execute()
{
    int deviceIndex = (ui.treeWidgetDevices->indexOfTopLevelItem(ui.treeWidgetDevices->currentItem()));

    if (type.at(deviceIndex) != "EN10MB")
    {
        // 3 - critical
        emit infoMessage(3, tr("Selecting device"), tr("No device selected. This program supports only Ethernet devices."));

        return false;
    }
    else
    {
        // jump to the selected device
        int i;
        for (i = 0, device = allDevices; i < deviceIndex; device = device->next, ++i);

        // we don't need any more the device list, free it
        //pcap_freealldevs(allDevices);
        return true;
    }
}

void DevicesDialog::onOK()
{
    int deviceIndex = (ui.treeWidgetDevices->indexOfTopLevelItem(ui.treeWidgetDevices->currentItem()));

    if (type.at(deviceIndex) != "EN10MB")
    {
        QMessageBox::information(this, tr("Information"), tr("This program supports only Ethernet devices!\nPlease select another device."));

        return;
    }
    else
    {
        // jump to the selected device
        int i;
        for (i = 0, device = allDevices; i < deviceIndex; device = device->next, ++i);

        // we don't need any more the device list, free it
        //pcap_freealldevs(allDevices);

        accept();
        close();
    }
}
