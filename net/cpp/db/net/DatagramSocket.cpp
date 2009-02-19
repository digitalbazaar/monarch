/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/DatagramSocket.h"

using namespace db::io;
using namespace db::net;

DatagramSocket::DatagramSocket()
{
}

DatagramSocket::~DatagramSocket()
{
}

inline bool DatagramSocket::bind(InternetAddress* address)
{
   return UdpSocket::bind(address);
}

inline bool DatagramSocket::joinGroup(
   InternetAddress* group, InternetAddress* localAddress)
{
   return UdpSocket::joinGroup(group, localAddress);
}

inline bool DatagramSocket::leaveGroup(InternetAddress* group)
{
   return UdpSocket::leaveGroup(group);
}

inline bool DatagramSocket::send(Datagram* datagram)
{
   ByteBuffer* buffer = datagram->getBuffer();
   return UdpSocket::sendDatagram(
      buffer->data(), buffer->length(), datagram->getAddress());
}

inline bool DatagramSocket::receive(Datagram* datagram)
{
   bool rval = false;
   
   // clear buffer and receive datagram
   ByteBuffer* buffer = datagram->getBuffer();
   buffer->clear();
   int size = UdpSocket::receiveDatagram(
      buffer->data(), buffer->freeSpace(), datagram->getAddress());
   if(size != -1)
   {
      // extend buffer to fill size of received datagram
      datagram->getBuffer()->extend(size);
      rval = true;
   }
   
   return rval;
}

inline bool DatagramSocket::setMulticastHops(unsigned char hops)
{
   return UdpSocket::setMulticastHops(hops);
}

inline bool DatagramSocket::setMulticastTimeToLive(unsigned char ttl)
{
   return UdpSocket::setMulticastTimeToLive(ttl);
}

inline bool DatagramSocket::setBroadcastEnabled(bool enable)
{
   return UdpSocket::setBroadcastEnabled(enable);
}

inline void DatagramSocket::close()
{
   UdpSocket::close();
}

inline bool DatagramSocket::isBound()
{
   return UdpSocket::isBound();
}

inline bool DatagramSocket::isConnected()
{
   return UdpSocket::isConnected();
}

inline bool DatagramSocket::getLocalAddress(InternetAddress* address)
{
   return UdpSocket::getLocalAddress(address);
}

inline void DatagramSocket::setSendTimeout(uint32_t timeout)
{
   UdpSocket::setSendTimeout(timeout);
}

inline uint32_t DatagramSocket::getSendTimeout()
{
   return UdpSocket::getSendTimeout();
}

inline void DatagramSocket::setReceiveTimeout(uint32_t timeout)
{
   UdpSocket::setReceiveTimeout(timeout);
}

inline uint32_t DatagramSocket::getReceiveTimeout()
{
   return UdpSocket::getReceiveTimeout();
}
