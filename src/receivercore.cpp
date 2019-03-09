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

#include "receivercore.h"

ReceiverCore::ReceiverCore(QObject *parent, CaptureThread *thread)
    : QObject(parent)
{
    connect(thread, SIGNAL(receivedPacket(QString,quint32,QString,QString,quint16,quint32,quint32,quint32,quint32,QString)), this, SLOT(receivedPacket(QString,quint32,QString,QString,quint16,quint32,quint32,quint32,quint32,QString)), Qt::QueuedConnection);

    connect(thread, SIGNAL(threadStarted()), this, SLOT(start()));
    connect(thread, SIGNAL(threadStopped()), this, SLOT(stop()));

    clearVariables();

    refreshTimer = new QTimer(this);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(updateRefreshTimer()));
}

ReceiverCore::~ReceiverCore()
{
}

void ReceiverCore::start()
{
    clearVariables();
    loadPorts();

    refreshTimer->start(1000);
}

void ReceiverCore::stop()
{
    refreshTimer->stop();
}

void ReceiverCore::setData(quint32 netMask, quint32 pcIP)
{
    this->netMask = netMask;
    this->pcIP = pcIP;
}

void ReceiverCore::updateRefreshTimer()
{
    // NetPacketsGraphDialog
    emit signalNetPacketsSpeed(netTotal - netTotalPrev);
    netTotalPrev = netTotal;

    // NetPacketsDialog
    emit signalNetPackets(netTotal, netArp, netRarp, netIcmp, netIgmp, netUdp, netTcp, netOther);

    // NetTransferDialog
    emit signalNetTransfer(netUpTotal, netDownTotal);

    // NetTransferGraphDialog & NetTransferDialog
    emit signalNetSpeed(((netUpTotal - netUpTotalPrev) / 1024.0), ((netDownTotal - netDownTotalPrev) / 1024.0));
    netUpTotalPrev = netUpTotal;
    netDownTotalPrev = netDownTotal;

    // MainWindow
    emit signalUsersTransfer(usersUp, usersDown);

    for (int i = 0; i < usersDownSpeed.count(); ++i)
    {
        usersUpSpeed[i] = (usersUp.at(i) - usersUpPrev.at(i)) / 1024.0;
        usersDownSpeed[i] = (usersDown.at(i) - usersDownPrev.at(i)) / 1024.0;

        usersUpPrev[i] = usersUp.at(i);
        usersDownPrev[i] = usersDown.at(i);
    }
    emit signalUsersSpeed(usersUpSpeed, usersDownSpeed);

    emit signalUsersApps(usersApps);
    emit signalUsersHosts(usersHosts);
    emit netAllPackets(usersArp, usersRarp, usersIcmp, usersIgmp, usersTcp, usersUdp, usersOther, usersTotal);
    emit netInPackets(usersArpIn, usersRarpIn, usersIcmpIn, usersIgmpIn, usersTcpIn, usersUdpIn, usersOtherIn, usersTotalIn);
    emit netOutPackets(usersArpOut, usersRarpOut, usersIcmpOut, usersIgmpOut, usersTcpOut, usersUdpOut, usersOtherOut, usersTotalOut);

    refreshTimer->start(1000);
}

void ReceiverCore::receivedPacket(const QString &time, quint32 length, const QString &sMac, const QString &dMac, quint16 type, quint32 sIP, quint32 dIP, quint32 sPort, quint32 dPort, const QString &info)
{
    incrementNetCounters(type);

    // IP from our network?
    if (checkIP(sIP))
    {
        if (checkIP(dIP))
        // network traffic
        {
        }
        // to Internet
        else
        {
            if (multicastIP(dIP))
            {
                // 0 download
                emit signalMulticast(dIP, sIP, length, 0);
                return;
            }

            if (!usersList.contains(sIP))
            {
                usersList.append(sIP);

                Hosts host;
                usersHosts.append(host);

                Apps app;
                usersApps.append(app);

                usersUpSpeed.append(0.0);
                usersDownSpeed.append(0.0);

                usersUp.append(0);
                usersUpPrev.append(0);
                usersDown.append(0);
                usersDownPrev.append(0);

                listsAppend();

                addr.S_un.S_addr = sIP;
                QString user = inet_ntoa(addr);

                emit signalNewUser(user, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

                QHostInfo::lookupHost(user, this, SLOT(userLookedUp(QHostInfo)));
            }

            int user = usersList.indexOf(sIP, 0);
            usersUp[user]+=length;

            netUpTotal+=length;

            if (!usersHosts.at(user).hostIp.contains(dIP))
            {
                usersHosts[user].hostIp.append(dIP);
                usersHosts[user].hostName.append("");

                addr.S_un.S_addr = dIP;
                QHostInfo::lookupHost(inet_ntoa(addr), this, SLOT(hostLookedUp(QHostInfo)));

                usersHosts[user].dPort.append(QString::number(dPort));
                usersHosts[user].dApp.append(portToName(dPort));
                usersHosts[user].downBytes.append(0);
                usersHosts[user].upBytes.append(0);
                usersHosts[user].firstVisit.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                usersHosts[user].lastVisit.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

                emit signalNewUserHost(user, usersHosts.at(user));
            }

            int index = usersHosts.at(user).hostIp.indexOf(dIP, 0);
            usersHosts[user].upBytes[index]+=length;
            usersHosts[user].lastVisit[index] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

            if (dPort != 0)
            {
                if (!usersApps.at(user).hostPort.contains(QString::number(dPort)))
                {
                    usersApps[user].hostPort.append(QString::number(dPort));
                    usersApps[user].hostPortName.append(portToName(dPort));
                    usersApps[user].upBytes.append(0);
                    usersApps[user].downBytes.append(0);

                    emit signalNewUserApp(user, usersApps.at(user));
                }
                usersApps[user].upBytes[usersApps.at(user).hostPort.indexOf(QString::number(dPort), 0)]+=length;
            }

            incrementOutLists(type, user);
        }
    }
    // from Internet
    else
    {
        if (multicastIP(sIP))
        {
            // 1 upload
            emit signalMulticast(sIP, dIP, length, 1);
            return;
        }

        if (multicastIP(dIP))
        {
            // 0 download
            emit signalMulticast(dIP, sIP, length, 0);
            return;
        }

        // to be sure that dIP is from our network
        if (checkIP(dIP))
        {
            // user
            if (!usersList.contains(dIP))
            {
                usersList.append(dIP);

                Hosts host;
                usersHosts.append(host);

                Apps app;
                usersApps.append(app);

                usersUpSpeed.append(0.0);
                usersDownSpeed.append(0.0);

                usersUp.append(0);
                usersUpPrev.append(0);
                usersDown.append(0);
                usersDownPrev.append(0);

                listsAppend();

                addr.S_un.S_addr = dIP;
                QString user = inet_ntoa(addr);

                emit signalNewUser(user, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

                QHostInfo::lookupHost(user, this, SLOT(userLookedUp(QHostInfo)));
            }

            int user = usersList.indexOf(dIP, 0);
            usersDown[user]+=length;

            netDownTotal+=length;

            if (!usersHosts.at(user).hostIp.contains(sIP))
            {
                usersHosts[user].hostIp.append(sIP);
                usersHosts[user].hostName.append("");

                addr.S_un.S_addr = sIP;
                QHostInfo::lookupHost(inet_ntoa(addr), this, SLOT(hostLookedUp(QHostInfo)));

                usersHosts[user].dPort.append(QString::number(sPort));
                usersHosts[user].dApp.append(portToName(sPort));
                usersHosts[user].downBytes.append(0);
                usersHosts[user].upBytes.append(0);
                usersHosts[user].firstVisit.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                usersHosts[user].lastVisit.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

                emit signalNewUserHost(user, usersHosts.at(user));
            }

            int index = usersHosts.at(user).hostIp.indexOf(sIP, 0);
            usersHosts[user].downBytes[index]+=length;
            usersHosts[user].lastVisit[index] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

            if (sPort != 0)
            {
                if (!usersApps.at(user).hostPort.contains(QString::number(sPort)))
                {
                    usersApps[user].hostPort.append(QString::number(sPort));
                    usersApps[user].hostPortName.append(portToName(sPort));
                    usersApps[user].upBytes.append(0);
                    usersApps[user].downBytes.append(0);

                    emit signalNewUserApp(user, usersApps.at(user));
                }
                usersApps[user].downBytes[usersApps.at(user).hostPort.indexOf(QString::number(sPort), 0)]+=length;
            }

            incrementInLists(type, user);
        }
    }
}

void ReceiverCore::clearVariables()
{
    usersList.clear();

    usersHosts.clear();
    usersApps.clear();

    usersUp.clear();
    usersDown.clear();
    usersUpPrev.clear();
    usersDownPrev.clear();;

    usersUpSpeed.clear();
    usersDownSpeed.clear();

    usersArp.clear();
    usersRarp.clear();
    usersIcmp.clear();
    usersIgmp.clear();
    usersTcp.clear();
    usersUdp.clear();
    usersOther.clear();
    usersTotal.clear();

    usersArpIn.clear();
    usersRarpIn.clear();
    usersIcmpIn.clear();
    usersIgmpIn.clear();
    usersTcpIn.clear();
    usersUdpIn.clear();
    usersOtherIn.clear();
    usersTotalIn.clear();

    usersArpOut.clear();
    usersRarpOut.clear();
    usersIcmpOut.clear();
    usersIgmpOut.clear();
    usersTcpOut.clear();
    usersUdpOut.clear();
    usersOtherOut.clear();
    usersTotalOut.clear();

    netUpTotal = 0;
    netDownTotal = 0;
    netUpTotalPrev = 0;
    netDownTotalPrev = 0;

    netTotalPrev = 0;
    netTotal = 0;
    netArp = 0;
    netRarp = 0;
    netIcmp = 0;
    netIgmp = 0;
    netUdp = 0;
    netTcp = 0;
    netOther = 0;
}

void ReceiverCore::incrementNetCounters(quint16 type)
{
    ++netTotal;

    if (type == 0x0806) { ++netArp; return; }
    if (type == 0x8035) { ++netRarp; return; }
    if (type == 1) { ++netIcmp; return; }
    if (type == 2) { ++netIgmp; return; }
    if (type == 6) { ++netTcp; return; }
    if (type == 17) { ++netUdp; return; }

    ++netOther;
}

// IP from our network?
bool ReceiverCore::checkIP(quint32 ip)
{
    for (int i = 0; i < 32; ++i)
    {
        if (netMask & 1<<i)
        {
            if ((ip & 1<<i) ^ (pcIP & 1<<i))
                return false;
        }
        else
        {
            return true;
        }
    }

    return true;
}

// is multicast IP?
bool ReceiverCore::multicastIP(quint32 ip)
{
    // 224.0.0.0 ... 239.255.255.255, RFC3171

    // reset (set 0) bits 8 to 32
    //for (int i = 8; i < 32; ++i)
    //    ip = ip & ~(1 << i);
    ip = ip & 0xff;
    
    if (ip >= 224 && ip <= 239)
        return true;
    else
        return false;

    return false;
}

void ReceiverCore::listsAppend()
{
    usersArp.append(0);
    usersRarp.append(0);
    usersIcmp.append(0);
    usersIgmp.append(0);
    usersTcp.append(0);
    usersUdp.append(0);
    usersOther.append(0);
    usersTotal.append(0);

    usersArpIn.append(0);
    usersRarpIn.append(0);
    usersIcmpIn.append(0);
    usersIgmpIn.append(0);
    usersTcpIn.append(0);
    usersUdpIn.append(0);
    usersOtherIn.append(0);
    usersTotalIn.append(0);

    usersArpOut.append(0);
    usersRarpOut.append(0);
    usersIcmpOut.append(0);
    usersIgmpOut.append(0);
    usersTcpOut.append(0);
    usersUdpOut.append(0);
    usersOtherOut.append(0);
    usersTotalOut.append(0);
}

void ReceiverCore::incrementInLists(quint16 type, qint32 i)
{
    ++usersTotalIn[i];
    ++usersTotal[i];

    if (type == 0x0806) { ++usersArpIn[i]; ++usersArp[i]; return; }
    if (type == 0x8035) { ++usersRarpIn[i]; ++usersRarp[i]; return; }
    if (type == 1) { ++usersIcmpIn[i]; ++usersIcmp[i]; return; }
    if (type == 2) { ++usersIgmpIn[i]; ++usersIgmp[i]; return; }
    if (type == 6) { ++usersTcpIn[i]; ++usersTcp[i]; return; }
    if (type == 17) { ++usersUdpIn[i]; ++usersUdp[i]; return; }

    ++usersOtherIn[i];
    ++usersOther[i];
}

void ReceiverCore::incrementOutLists(quint16 type, qint32 i)
{
    ++usersTotalOut[i];
    ++usersTotal[i];

    if (type == 0x0806) { ++usersArpOut[i]; ++usersArp[i]; return; }
    if (type == 0x8035) { ++usersRarpOut[i]; ++usersRarp[i]; return; }
    if (type == 1) { ++usersIcmpOut[i]; ++usersIcmp[i]; return; }
    if (type == 2) { ++usersIgmpOut[i]; ++usersIgmp[i]; return; }
    if (type == 6) { ++usersTcpOut[i]; ++usersTcp[i]; return; }
    if (type == 17) { ++usersUdpOut[i]; ++usersUdp[i]; return; }

    ++usersOtherOut[i];
    ++usersOther[i];
}

void ReceiverCore::loadPorts()
{
    protocolList.clear();
    portList.clear();
    descList.clear();

    QFile file(QCoreApplication::applicationDirPath() + "/ports.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // 2 - warning
        emit infoMessage(2, tr("Receiver core/thread"), tr("Unable to open port numbers file: %1. Users application names not available.").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    QString line;

    while (!in.atEnd())
    {
        line = in.readLine();

        //protocolList.append(line.left(3));
        portList.append(line.mid(4, line.indexOf("=", 0) - 4));
        descList.append(line.right(line.length() - line.indexOf("=", 3) -1).simplified());
    }

    file.close();
}

QString ReceiverCore::portToName(quint16 port)
{
    int index = portList.indexOf(QString::number(port), 0);

    if (index == -1) // port number isn't on list
        return "";
    else
        return descList.at(index);
}

void ReceiverCore::hostLookedUp(const QHostInfo &host)
{
    if (host.error() != QHostInfo::NoError)
    {
        // "Lookup failed:" << host.errorString();
        return;
    }

    for (int i = 0; i < usersHosts.count(); ++i)
        for (int j = 0; j < usersHosts.at(i).hostIp.count(); ++j)
            foreach (QHostAddress address, host.addresses())
            {
                hostAddr.S_un.S_addr = usersHosts.at(i).hostIp.at(j);

                if (inet_ntoa(hostAddr) == address.toString())
                {
                    usersHosts[i].hostName[j] = host.hostName();

                    emit signalNewHostName(address.toString(), host.hostName());
                }
            }
}

void ReceiverCore::userLookedUp(const QHostInfo &host)
{
    if (host.error() != QHostInfo::NoError)
    {
        // "Lookup failed:" << host.errorString();
        return;
    }

    for (int i = 0; i < usersList.count(); ++i)
    {
        userAddr.S_un.S_addr = usersList.at(i);
        QString user = inet_ntoa(userAddr);

        foreach (QHostAddress address, host.addresses())
        {
            if (user == address.toString())
            {
                if (QString::compare(user, host.hostName(), Qt::CaseSensitive) !=0)
                    emit signalNewUserName(user, host.hostName());
            }
        }
    }
}
