/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Socket.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

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

void Socket::bind(SocketAddress* address)
{
}

void Socket::accept(Socket* socket, unsigned int timeout)
{
}

void Socket::connect(SocketAddress* address, unsigned int timeout)
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

int Socket::read(char* b, int offset, int length)
{
   // FIXME:
   return -1;
}

void Socket::write(char* b, int offset, int length)
{
}
