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

#include "capturethread.h"

CaptureThread::CaptureThread(QObject *parent)
    : QThread(parent)
{
}

CaptureThread::~CaptureThread()
{
    abort = true;
    wait();
}

bool CaptureThread::startCapture(pcap_if_t *d, quint8 mode, quint16 bytes, quint16 timeout, const QString &filterCode, qint32 packetsLimit)
{
    abort = false;
    packets = 0;
    this->packetsLimit = packetsLimit; // -1 if no limit

    // open the device
    char errbuf[PCAP_ERRBUF_SIZE];
    if ((adhandle = pcap_open_live(d->name, bytes, mode, timeout, errbuf)) == NULL)
    {
        // 3 - critical
        emit infoMessage(3, tr("Capture thread"), tr("Unable to open the network device. Probably the selected device is not supported by WinPcap."));

        // free the device list
        //pcap_freealldevs(alldevs);

        return false;
    }

    // retrieve the mask
    u_int netmask;

    if (d->addresses != NULL)
    {
        // retrieve the mask of the first address of the interface
        netmask = ((struct sockaddr_in*)(d->addresses->netmask))->sin_addr.S_un.S_addr;
    }
    else
    {
        // if the interface is without addresses we suppose to be in a C class network
        netmask = 0xffffff;
    }

    // compile the filter
    struct bpf_program fcode;

    if (pcap_compile(adhandle, &fcode, filterCode.toLocal8Bit().data(), 1, netmask) < 0 )
    {
        // 3 - critical
        emit infoMessage(3, tr("Capture thread"), tr("Unable to compile the packet filter. Check the syntax."));

        // free the device list
        //pcap_freealldevs(alldevs);

        return false;
    }

    // set the filter
    if (pcap_setfilter(adhandle, &fcode) < 0)
    {
        // 3 - critical
        emit infoMessage(3, tr("Capture thread"), tr("Error setting the filter."));

        // free the device list
        //pcap_freealldevs(alldevs);

        return false;
    }

    if (!isRunning())
        start(NormalPriority);

    if (isRunning())
    {
        emit threadStarted();

        return true;
    }
    else
    {
        return false;
    }
}

bool CaptureThread::stopCapture()
{
    abort = true;
    wait();

    emit threadStopped();

    return true;
}

void CaptureThread::run()
{
    eth_header *ethHeader;
    arp_header *arpHeader;
    rarp_header *rarpHeader;
    ip_header *ipHeader;
    tcp_header *tcpHeader;
    udp_header *udpHeader;
    icmp_header *icmpHeader;
    igmp_header *igmpHeader;

    char tcpFlag[8][5] = {"FIN ", "SYN ", "RST ", "PSH ", "ACK ", "URG ", "ECE ", "CWR "};

    int icmp_mesglen = 16;
    icmp_mesg icmpMesg[] = { {0, "Echo Reply"},
                             {3, "Destination Unreachable"},
                             {4, "Source Quench"},
                             {5, "Redirect Message"},
                             {6, "Alternate Host Address"},
                             {8, "Echo Request"},
                             {9, "Router Advertisement"},
                             {10, "Router Selection"},
                             {11, "Time Exceeded"},
                             {12, "Parameter Problem"},
                             {13, "Timestamp Request"},
                             {14, "Timestamp Reply"},
                             {15, "Information Request"},
                             {16, "Information Reply"},
                             {17, "Address Mask Request"},
                             {18, "Address Mask Reply"}
                           };

    int igmp_mesglen = 8;
    igmp_mesg igmpMesg[] = { {0x11, "Membership Query"},
                             {0x12, "IGMPv1 Membership Report"},
                             {0x16, "IGMPv2 Membership Report"},
                             {0x17, "Leave Group"},
                             {0x22, "IGMPv3 Membership Report"},
                             {0x24, "Multicast Router Advertisement"},
                             {0x25, "Multicast Router Solicitation"},
                             {0x26, "Multicast Router Termination"}
                           };

    struct pcap_pkthdr *header;
    const u_char *pkt_data;
    time_t local_tv_sec;
    struct tm *ltime;
    char timeStr[16];
    QString info;
    char sMac[18], dMac[18], source[20], dest[20];
    u_int ip_hlen;
    int i, res;

    // retrieve the packets
    forever
    {
        if (abort)
            return;

        res = pcap_next_ex(adhandle, &header, &pkt_data);

        // The return value can be:
        //  1 if the packet has been read without problems
        //  0 if the timeout set with pcap_open_live() has elapsed, in this case pkt_header and pkt_data don't point to a valid packet
        // -1 if an error occurred
        // -2 if EOF was reached reading from an offline capture

        if (res == -1)
        {
            // 3 - critical
            emit infoMessage(3, tr("Capture thread"), QString(tr("Error while reading packet: \"%1\"")).arg(pcap_geterr(adhandle)));
            emit breakThread();
            return;
    	}

        if (res == 0)
            continue;

        if (packets == packetsLimit)
        {
            emit breakThread();
            return;
        }
        ++packets;

        // convert the timestamp to readable format
        local_tv_sec = header->ts.tv_sec;
        ltime = localtime(&local_tv_sec);
        strftime(timeStr, sizeof timeStr, "%H:%M:%S", ltime);

// ETH
        ethHeader = (eth_header*)pkt_data;

        sprintf(sMac, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", ethHeader->smac[0], ethHeader->smac[1], ethHeader->smac[2], ethHeader->smac[3], ethHeader->smac[4], ethHeader->smac[5]);
        sprintf(dMac, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", ethHeader->dmac[0], ethHeader->dmac[1], ethHeader->dmac[2], ethHeader->dmac[3], ethHeader->dmac[4], ethHeader->dmac[5]);

// ARP
        // 0x0806 Address Resolution Protocol (ARP)
    	if (ntohs(ethHeader->type) == 0x0806)
    	{
            arpHeader = (arp_header*)(pkt_data + ETHERNET_LENGTH);

            sprintf(source, "%d.%d.%d.%d", arpHeader->spa.byte1, arpHeader->spa.byte2, arpHeader->spa.byte3, arpHeader->spa.byte4);
            sprintf(dest, "%d.%d.%d.%d", arpHeader->tpa.byte1, arpHeader->tpa.byte2, arpHeader->tpa.byte3, arpHeader->tpa.byte4);

            // 1 ARP request
            // 2 ARP response
            // 3 RARP request
            // 4 RARP response
            // 5 Dynamic RARP request
            // 6 Dynamic RARP reply
            // 7 Dynamic RARP error
            // 8 InARP request
            // 9 InARP reply

            if (ntohs(arpHeader->oper) == 0x0001)
                info = "ARP request";

            if (ntohs(arpHeader->oper) == 0x0002)
                info = "ARP response";

            emit receivedPacket(timeStr + QString(".%1").arg(header->ts.tv_usec),
                                header->len,
                                sMac,
                                dMac,
                                0x0806,
                                inet_addr(source),
                                inet_addr(dest),
                                65536,
                                65536,
                                info);
            continue;
        }

// RARP
        // 0x8035 Reverse Address Resolution Protocol (RARP)
    	if (ntohs(ethHeader->type) == 0x8035)
    	{
            rarpHeader = (rarp_header*)(pkt_data + ETHERNET_LENGTH);

            sprintf(source, "%d.%d.%d.%d", rarpHeader->spa.byte1, rarpHeader->spa.byte2, rarpHeader->spa.byte3, rarpHeader->spa.byte4);
            sprintf(dest, "%d.%d.%d.%d", rarpHeader->tpa.byte1, rarpHeader->tpa.byte2, rarpHeader->tpa.byte3, rarpHeader->tpa.byte4);

            // 1 ARP request
            // 2 ARP response
            // 3 RARP request
            // 4 RARP response
            // 5 Dynamic RARP request
            // 6 Dynamic RARP reply
            // 7 Dynamic RARP error
            // 8 InARP request
            // 9 InARP reply

            if (ntohs(rarpHeader->oper) == 0x0003)
                info = "RARP request";

            if (ntohs(rarpHeader->oper) == 0x0004)
                info = "RARP response";

            emit receivedPacket(timeStr + QString(".%1").arg(header->ts.tv_usec),
                                header->len,
                                sMac,
                                dMac,
                                0x8035,
                                inet_addr(source),
                                inet_addr(dest),
                                65536,
                                65536,
                                info);
            continue;
        }

// IP  	// 0x0800 Internet Protocol, Version 4 (IPv4) //0x86DD 	Internet Protocol, Version 6 (IPv6)
        if (ntohs(ethHeader->type) == 0x0800)
    	{
            ipHeader = (ip_header*)(pkt_data + ETHERNET_LENGTH);

            // Internet Header Length is the length of the internet header in 32
            // bit words, and thus points to the beginning of the data.
            // Note that the minimum value for a correct header is 5 (5×32 = 160 bits).
            // Being a 4-bit value, the maximum length is 15 words (15×32 bits) or 480 bits.

            ip_hlen = (ipHeader->ver_ihl & 0xf) << 2;

            switch (ipHeader->proto)
            {
// IP TCP
                case 6: tcpHeader = (tcp_header*)((u_char*)ipHeader + ip_hlen);

                        for (i = 0, info = ""; i < 8; ++i)
                        {
                            if (tcpHeader->flag & 1<<i)
                                info.append(tcpFlag[i]);
                        }

                        emit receivedPacket(timeStr + QString(".%1").arg(header->ts.tv_usec),
                                            header->len,
                                            sMac,
                                            dMac,
                                            6,
                                            ipHeader->saddr,
                                            ipHeader->daddr,
                                            ntohs(tcpHeader->sport),
                                            ntohs(tcpHeader->dport),
                                            info);
                        break;
// IP UDP
                case 17: udpHeader = (udp_header*)((u_char*)ipHeader + ip_hlen); //jak wyzej

                         emit receivedPacket(timeStr + QString(".%1").arg(header->ts.tv_usec),
                                             header->len,
                                             sMac,
                                             dMac,
                                             17,
                                             ipHeader->saddr,
                                             ipHeader->daddr,
                                             ntohs(udpHeader->sport),
                                             ntohs(udpHeader->dport),
                                             "");
                         break;
// IP ICMP
                case 1: icmpHeader = (icmp_header*)((u_char*)ipHeader + ip_hlen);//^

                        for (i = 0; i < icmp_mesglen; ++i)
                        {
                            if (icmpHeader->type == icmpMesg[i].type)
                            {
                                info = icmpMesg[i].mesg;
                                break;
                            }
                        }

                        if (i == icmp_mesglen)
                            info = "unknown ICMP message type";

                        emit receivedPacket(timeStr + QString(".%1").arg(header->ts.tv_usec),
                                            header->len,
                                            sMac,
                                            dMac,
                                            1,
                                            ipHeader->saddr,
                                            ipHeader->daddr,
                                            65536,
                                            65536,
                                            info);
                        break;
// IP IGMP
                case 2: igmpHeader = (igmp_header*)((u_char*)ipHeader + ip_hlen);//tak jak wyzej

                        for (i = 0; i < igmp_mesglen; ++i)
                        {
                            if (igmpHeader->type == igmpMesg[i].type)
                            {
                                info = igmpMesg[i].mesg;
                                break;
                            }
                        }

                        if (i == igmp_mesglen)
                            info = "unknown IGMP message type";

                        emit receivedPacket(timeStr + QString(".%1").arg(header->ts.tv_usec),
                                            header->len,
                                            sMac,
                                            dMac,
                                            2,
                                            ipHeader->saddr,
                                            ipHeader->daddr,
                                            65536,
                                            65536,
                                            info);
                        break;
// other
                default: info = "protocol not supported";

                         emit receivedPacket(timeStr + QString(".%1").arg(header->ts.tv_usec),
                                             header->len,
                                             sMac,
                                             dMac,
                                             0,
                                             ipHeader->saddr,
                                             ipHeader->daddr,
                                             65536,
                                             65536,
                                             info);
                        break;
            }
            continue;
        }

        info = "protocol not supported";
        emit receivedPacket(timeStr + QString(".%1").arg(header->ts.tv_usec),
                            header->len,
                            sMac,
                            dMac,
                            ntohs(ethHeader->type),
                            0,
                            0,
                            65536,
                            65536,
                            info);
    }
}
