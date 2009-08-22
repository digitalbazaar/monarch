/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
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

bool DatagramSocket::send(DatagramRef& datagram)
{
   ByteBuffer* buffer = datagram->getBuffer();
   return UdpSocket::sendDatagram(
      buffer->data(), buffer->length(), &(*datagram->getAddress()));
}

bool DatagramSocket::receive(DatagramRef& datagram)
{
   bool rval = false;

   // clear buffer and receive datagram
   ByteBuffer* buffer = datagram->getBuffer();
   buffer->clear();
   int size = UdpSocket::receiveDatagram(
      buffer->data(), buffer->freeSpace(), &(*datagram->getAddress()));
   if(size != -1)
   {
      // extend buffer to fill size of received datagram
      datagram->getBuffer()->extend(size);
      rval = true;
   }

   return rval;
}
