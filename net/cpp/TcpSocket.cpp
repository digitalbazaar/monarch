/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "TcpSocket.h"

using namespace db::io;
using namespace db::net;

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

void TcpSocket::initialize() throw(SocketException)
{
   if(mFileDescriptor == -1)
   {
      create(SOCK_STREAM, IPPROTO_TCP);
   }
}

Socket* TcpSocket::createConnectedSocket(unsigned int fd) throw(SocketException)
{
   // create a new TcpSocket
   TcpSocket* socket = new TcpSocket();
   socket->mFileDescriptor = fd;
   socket->mBound = true;
   socket->mConnected = true;
   
   return socket;
}
