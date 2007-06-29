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

bool DatagramSocket::bind(InternetAddress* address)
{
   return UdpSocket::bind(address);
}

bool DatagramSocket::joinGroup(
   InternetAddress* group, InternetAddress* localAddress)
{
   return UdpSocket::joinGroup(group, localAddress);
}

bool DatagramSocket::leaveGroup(InternetAddress* group)
{
   return UdpSocket::leaveGroup(group);
}

bool DatagramSocket::send(Datagram* datagram)
{
   unsigned int length = 0;
   char* data = datagram->getData(length);
   return UdpSocket::sendDatagram(data, length, datagram->getAddress());
}

bool DatagramSocket::receive(Datagram* datagram)
{
   bool rval = false;
   
   unsigned int length = 0;
   char* data = datagram->getData(length);
   int size = UdpSocket::receiveDatagram(data, length, datagram->getAddress());
   if(size != -1)
   {
      datagram->setLength(size);
      rval = true;
   }
   
   return rval;
}

bool DatagramSocket::setMulticastHops(unsigned char hops)
{
   return UdpSocket::setMulticastHops(hops);
}

bool DatagramSocket::setMulticastTimeToLive(unsigned char ttl)
{
   return UdpSocket::setMulticastTimeToLive(ttl);
}

bool DatagramSocket::setBroadcastEnabled(bool enable)
{
   return UdpSocket::setBroadcastEnabled(enable);
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

bool DatagramSocket::getLocalAddress(InternetAddress* address)
{
   return UdpSocket::getLocalAddress(address);
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
