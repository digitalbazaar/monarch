/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DatagramSocket.h"

using namespace db::net;

DatagramSocket::DatagramSocket()
{
}

DatagramSocket::~DatagramSocket()
{
}

void DatagramSocket::bind(InternetAddress* address) throw(SocketException)
{
   return UdpSocket::bind(address);
}

void DatagramSocket::joinGroup(
   InternetAddress* group, InternetAddress* localAddress) throw(SocketException)
{
   UdpSocket::joinGroup(group, localAddress);
}

void DatagramSocket::leaveGroup(InternetAddress* group) throw(SocketException)
{
   UdpSocket::leaveGroup(group);
}

void DatagramSocket::send(Datagram* datagram) throw(db::io::IOException)
{
   unsigned int length = 0;
   char* data = datagram->getData(length);
   UdpSocket::sendDatagram(data, length, datagram->getAddress());
}

void DatagramSocket::receive(Datagram* datagram) throw(db::io::IOException)
{
   unsigned int length = 0;
   char* data = datagram->getData(length);
   length = UdpSocket::receiveDatagram(data, length, datagram->getAddress());
   datagram->setLength(length);
}

void DatagramSocket::setMulticastHops(unsigned char hops) throw(SocketException)
{
   UdpSocket::setMulticastHops(hops);
}

void DatagramSocket::setMulticastTimeToLive(unsigned char ttl)
throw(SocketException)
{
   UdpSocket::setMulticastTimeToLive(ttl);
}

void DatagramSocket::setBroadcastEnabled(bool enable) throw(SocketException)
{
   UdpSocket::setBroadcastEnabled(enable);
}

void DatagramSocket::close()
{
   UdpSocket::close();
}

bool DatagramSocket::isBound()
{
   return UdpSocket::isBound();
}

bool DatagramSocket::isConnected()
{
   return UdpSocket::isConnected();
}

void DatagramSocket::getLocalAddress(InternetAddress* address)
throw(SocketException)
{
   UdpSocket::getLocalAddress(address);
}

void DatagramSocket::setSendTimeout(unsigned long long timeout)
{
   UdpSocket::setSendTimeout(timeout);
}

unsigned long long DatagramSocket::getSendTimeout()
{
   return UdpSocket::getSendTimeout();
}

void DatagramSocket::setReceiveTimeout(unsigned long long timeout)
{
   UdpSocket::setReceiveTimeout(timeout);
}

unsigned long long DatagramSocket::getReceiveTimeout()
{
   return UdpSocket::getReceiveTimeout();
}
