/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SslSocket.h"

using namespace db::net;

SslSocket::SslSocket(TcpSocket* socket, bool cleanup) :
   SocketWrapper(socket, cleanup)
{
}

SslSocket::~SslSocket()
{
}
