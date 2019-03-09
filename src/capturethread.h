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

#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>

#include "WpdPack/Include/pcap.h"

#include "protocols.h"

class CaptureThread : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(CaptureThread);

public:
    explicit CaptureThread(QObject *parent = 0);
    ~CaptureThread();

    bool startCapture(pcap_if_t *d, quint8 mode, quint16 bytes, quint16 timeout, const QString &filterCode, qint32 packetsLimit);
    bool stopCapture();

protected:
    virtual void run();

private:
    bool abort;

    quint64 packets;
    qint32 packetsLimit;

    pcap_t *adhandle;

signals:
    void infoMessage(quint8 type, const QString &title, const QString &message);

    void breakThread();

    void threadStarted();
    void threadStopped();

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
};
#endif // CAPTURETHREAD_H
