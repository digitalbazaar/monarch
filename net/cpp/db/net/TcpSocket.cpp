/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/TcpSocket.h"
#include "db/net/SocketDefinitions.h"
#include "db/io/PeekInputStream.h"
#include "db/net/SocketInputStream.h"
#include "db/net/SocketOutputStream.h"

using namespace db::io;
using namespace db::net;

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

bool TcpSocket::acquireFileDescriptor(const char* domain)
{
   bool rval = true;
   
   if(mFileDescriptor == -1)
   {
      // use PF_INET = "protocol family internet" (which just so happens to
      // have the same value as AF_INET but that's only because different
      // protocols were never used with the same address family
      if(strcmp(domain, "IPv6") == 0)
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

Socket* TcpSocket::createConnectedSocket(unsigned int fd)
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
