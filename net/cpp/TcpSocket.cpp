/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "TcpSocket.h"
#include "SocketDefinitions.h"
#include "PeekInputStream.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

using namespace std;
using namespace db::io;
using namespace db::net;

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

void TcpSocket::acquireFileDescriptor(const string& domain)
throw(SocketException)
{
   if(mFileDescriptor == -1)
   {
      // use PF_INET = "protocol family internet" (which just so happens to
      // have the same value as AF_INET but that's only because different
      // protocols were never used with the same address family
      if(domain == "IPv6")
      {
         // use IPv6
         create(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
      }
      else
      {
         // default to IPv4
         create(PF_INET, SOCK_STREAM, IPPROTO_TCP);
      }
   }
}

Socket* TcpSocket::createConnectedSocket(unsigned int fd) throw(SocketException)
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
