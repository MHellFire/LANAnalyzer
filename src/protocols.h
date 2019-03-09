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

#ifndef PROTOCOLS_H
#define PROTOCOLS_H

// Ethernet type II header length
const quint8 ETHERNET_LENGTH = 14;

// Ethernet type II header
struct eth_header
{
    quint8 dmac[6];     // destination mac address
    quint8 smac[6]; 	// source mac address
    quint16 type;   	// EtherType: IP, ARP, RARP...
};

// IPv4 address
struct ip_address
{
    quint8 byte1;
    quint8 byte2;
    quint8 byte3;
    quint8 byte4;
};

// ARP header
struct arp_header
{
    quint16 htype;	// hardware type, Ethernet is 1
    quint16 ptype;  	// protocol type, IPv4 is 0x0800
    quint8 hlen;    	// hardware length, Ethernet addresses size is 6
    quint8 plen;    	// protocol length, IPv4 address size is 4
    quint16 oper;    	// operation, request = 1, reply = 2
    quint8 smac[6];    	// sender hardware address
    ip_address spa;  	// sender protocol address
    quint8 dmac[6];    	// target hardware address
    ip_address tpa;     // target protocol address
};

typedef arp_header rarp_header;

// IPv4 header
struct ip_header
{
    quint8 ver_ihl;             // version (4 bits) + internet header length (4 bits)
    quint8 tos;     	        // type of service
    quint16 tlen;          	// total length
    quint16 identification; 	// identification
    quint16 flags_fo;       	// flags (3 bits) + fragment offset (13 bits)
    quint8 ttl;            	// time to live
    quint8 proto;          	// protocol
    quint16 crc;           	// header checksum
    quint32 saddr;      	// IPv4 source address
    quint32 daddr;      	// IPv4 destination address
    //quint32 op_pad;         	// option + padding
};

// TCP header
struct tcp_header
{
    quint16 sport;      // source port
    quint16 dport;      // destination port
    quint32 seqno;      // sequence number
    quint32 ackno;      // acknowledgment number
    quint8 offset;      // data offset
    quint8 flag;        // flags
    quint16 win;        // window
    quint16 checksum;   // crc
    quint16 uptr;	// urgent pointer
};

// UDP header
struct udp_header
{
    quint16 sport;          // source port
    quint16 dport;          // destination port
    quint16 len;            // length
    quint16 checksum;       // checksum
};

// ICMP header
struct icmp_header
{
    quint8 type;        // type
    quint8 code;	// code
    quint16 checksum;	// checksum
    quint16 id;		// id
    quint16 seqno;	// sequence
};

// ICMP message
struct icmp_mesg
{
    quint8 type;	// type
    QString mesg;       // message
};

// IGMPv2
struct igmp_header
{
    quint8 type;         // type
    quint8 restime;      // max resp time
    quint16 checksum;    // checksum
    quint32 groupaddr;   // group address
};

// IGMP message
struct igmp_mesg
{
    quint8 type;    // type
    QString mesg;   // message
};

#endif // PROTOCOLS_H
