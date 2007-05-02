/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "UdpSocket.h"

using namespace db::io;
using namespace db::net;

UdpSocket::UdpSocket()
{
}

UdpSocket::~UdpSocket()
{
}

void UdpSocket::initialize() throw(SocketException)
{
   if(mFileDescriptor == -1)
   {
      create(SOCK_DGRAM, IPPROTO_UDP);
   }
}

Socket* UdpSocket::createConnectedSocket(unsigned int fd) throw(SocketException)
{
   // create a new UdpSocket
   UdpSocket* socket = new UdpSocket();
   socket->mFileDescriptor = fd;
   socket->mBound = true;
   socket->mConnected = true;
   
   return socket;
}
