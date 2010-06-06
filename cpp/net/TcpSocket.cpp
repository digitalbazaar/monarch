/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/TcpSocket.h"

#include "monarch/net/SocketDefinitions.h"
#include "monarch/io/PeekInputStream.h"
#include "monarch/net/SocketInputStream.h"
#include "monarch/net/SocketOutputStream.h"

#include <cstring>

using namespace monarch::io;
using namespace monarch::net;

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

bool TcpSocket::acquireFileDescriptor(SocketAddress::CommunicationDomain domain)
{
   bool rval = true;

   if(mFileDescriptor == -1)
   {
      // use PF_INET = "protocol family internet" (which just so happens to
      // have the same value as AF_INET but that's only because different
      // protocols were never used with the same address family
      if(domain == SocketAddress::IPv6)
      {
         // use IPv6
         rval = create(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
      }
      else
      {
         // default to IPv4
         rval = create(PF_INET, SOCK_STREAM, IPPROTO_TCP);
      }
   }

   return rval;
}

Socket* TcpSocket::createConnectedSocket(int fd)
{
   // create a new TcpSocket
   TcpSocket* socket = new TcpSocket();
   socket->mFileDescriptor = fd;
   socket->mBound = true;
   socket->mConnected = true;

   // initialize input and output
   socket->initializeInput();
   socket->initializeOutput();

   return socket;
}
