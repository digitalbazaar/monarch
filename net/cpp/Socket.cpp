/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Socket.h"

using namespace db::io;
using namespace db::net;

Socket::Socket() : mInputStream(this), mOutputStream(this)
{
   // file descriptor is invalid at this point
   mFileDescriptor = -1;
}

Socket::~Socket()
{
   // close socket
   close();
}

void Socket::initialize()
{
}

void Socket::bind(SocketAddress* address) throw(SocketException)
{
}

void Socket::accept(Socket* socket, unsigned int timeout)
throw(SocketException, SocketTimeoutException)
{
}

void Socket::connect(SocketAddress* address, unsigned int timeout)
throw(SocketException)
{
}

void Socket::close()
{
}

SocketInputStream& Socket::getInputStream()
{
   return mInputStream;
}

SocketOutputStream& Socket::getOutputStream()
{
   return mOutputStream;
}

int Socket::receive(char* b, int offset, int length) throw(IOException)
{
   // FIXME:
   return -1;
}

void Socket::send(char* b, int offset, int length) throw(IOException)
{
}
