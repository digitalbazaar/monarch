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

void TcpSocket::bind(SocketAddress* address) throw(SocketException)
{
}

void TcpSocket::accept(Socket* socket, unsigned int timeout)
throw(SocketException, SocketTimeoutException)
{
}

void TcpSocket::connect(SocketAddress* address, unsigned int timeout)
throw(SocketException)
{
}
