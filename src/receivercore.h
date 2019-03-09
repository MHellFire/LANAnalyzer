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

#ifndef RECEIVERCORE_H
#define RECEIVERCORE_H

#include <QObject>
#include <QCoreApplication>
#include <QHostInfo>
#include <QTimer>
#include <QDateTime>
#include <QMetaType>
#include <QFile>

#include "capturethread.h"

struct Hosts
{
    QList<quint32> hostIp;
    QList<QString> hostName;
    QList<QString> dPort;
    QList<QString> dApp;
    QList<quint64> upBytes;
    QList<quint64> downBytes;
    QList<QString> firstVisit;
    QList<QString> lastVisit;
};

typedef QList<Hosts> hostsList;

struct Apps
{
    QList<QString> hostPort;
    QList<QString> hostPortName;
    QList<quint64> upBytes;
    QList<quint64> downBytes;
};


typedef QList<Apps> appsList;

typedef QList<quint32> quint32List;
typedef QList<quint64> quint64List;
typedef QList<qreal> qrealList;

class ReceiverCore : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ReceiverCore);

public:
    explicit ReceiverCore(QObject *parent = 0, CaptureThread *thread = 0);
    ~ReceiverCore();

    void setData(quint32 netMask, quint32 pcIP);

private:
    QTimer *refreshTimer;

    quint32 netMask, pcIP;
    struct in_addr addr, userAddr, hostAddr;

    // users
    QList<quint32> usersList;

    QList<Hosts> usersHosts;
    QList<Apps> usersApps;

    QList<quint64> usersUp, usersDown,
                   usersUpPrev, usersDownPrev;
    QList<qreal> usersUpSpeed, usersDownSpeed;

    QList<quint32> usersArp, usersRarp, usersIcmp, usersIgmp, usersTcp, usersUdp, usersOther, usersTotal,
                   usersArpIn, usersRarpIn, usersIcmpIn, usersIgmpIn, usersTcpIn, usersUdpIn, usersOtherIn, usersTotalIn,
                   usersArpOut, usersRarpOut, usersIcmpOut, usersIgmpOut, usersTcpOut, usersUdpOut, usersOtherOut, usersTotalOut;

    // network
    quint64 netUpTotal, netDownTotal,
            netUpTotalPrev, netDownTotalPrev;

    quint64 netTotal,
            netArp,
            netRarp,
            netIcmp,
            netIgmp,
            netUdp,
            netTcp,
            netOther;
    quint64 netTotalPrev;

    // ports
    QList<QString> protocolList, portList, descList;

    void clearVariables();

    void incrementNetCounters(quint16 type);
    void incrementInLists(quint16 type, qint32 i);
    void incrementOutLists(quint16 type, qint32 i);

    bool checkIP(quint32 ip);
    bool multicastIP(quint32 ip);

    void listsAppend();

    void loadPorts();

    QString portToName(quint16 port);

private slots:
    void receivedPacket(const QString &time,
                        quint32 length,
                        const QString &sMac,
                        const QString &dMac,
                        quint16 type,
                        quint32 sIP,
                        quint32 dIP,
                        quint32 sPort,
                        quint32 dPort,
                        const QString &info);

    void updateRefreshTimer();

    void hostLookedUp(const QHostInfo &host);
    void userLookedUp(const QHostInfo &host);

    void start();
    void stop();

signals:
    void infoMessage(quint8 type, const QString &title, const QString &message);

    void signalMulticast(quint32 multicastIP, quint32 otherIP, quint32 length, quint8 direction);

    void signalNetPackets(quint64 netTotal, quint64 netArp, quint64 netRarp, quint64 netIcmp, quint64 netIgmp, quint64 netUdp, quint64 netTcp, quint64 netOther);
    void signalNetPacketsSpeed(quint16 packetsSpeed);
    void signalNetTransfer(quint64 up, quint64 down);
    void signalNetSpeed(const qreal &upSpeed, const qreal &downSpeed);

    void signalNewUser(const QString &user, const QString &timeOn);
    void signalNewUserName(const QString &user, const QString &name);

    void signalUsersTransfer(QList<quint64> usersUp, QList<quint64> usersDown);
    void signalUsersSpeed(QList<qreal> usersUpSpeed, QList<qreal> usersDownSpeed);

    void netAllPackets(QList<quint32> userArp, QList<quint32> userRarp, QList<quint32> userIcmp, QList<quint32> userIgmp, QList<quint32> userTcp, QList<quint32> userUdp, QList<quint32> userOther, QList<quint32> userTotal);
    void netInPackets(QList<quint32> userArpIn, QList<quint32> userRarpIn, QList<quint32> userIcmpIn, QList<quint32> userIgmpIn, QList<quint32> userTcpIn, QList<quint32> userUdpIn, QList<quint32> userOtherIn, QList<quint32> userTotalIn);
    void netOutPackets(QList<quint32> userArpOut, QList<quint32> userRarpOut, QList<quint32> userIcmpOut, QList<quint32> userIgmpOut, QList<quint32> userTcpOut, QList<quint32> userUdpOut, QList<quint32> userOtherOut, QList<quint32> userTotalOut);

    void signalNewUserApp(quint16 user, Apps app);
    void signalNewUserHost(quint16 user, Hosts host);
    void signalNewHostName(const QString &hostAddress, const QString &hostName);

    void signalUsersApps(QList<Apps> usersApps);
    void signalUsersHosts(QList<Hosts> usersHosts);
};

#endif // RECEIVERCORE_H
