/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketWrapper.h"

using namespace db::io;
using namespace db::net;

SocketWrapper::SocketWrapper(Socket* socket, bool cleanup)
{
   setSocket(socket, cleanup);
}

SocketWrapper::~SocketWrapper()
{
   // cleanup wrapped socket as appropriate
   if(mCleanupSocket && getSocket() != NULL)
   {
      delete getSocket();
   }
}

void SocketWrapper::setSocket(Socket* socket, bool cleanup)
{
   mSocket = socket;
   mCleanupSocket = cleanup;
}

Socket* SocketWrapper::getSocket()
{
   return mSocket;
}

void SocketWrapper::bind(SocketAddress* address) throw(SocketException)
{
   getSocket()->bind(address);
}

void SocketWrapper::listen(unsigned int backlog) throw(SocketException)
{
   getSocket()->listen(backlog);
}

Socket* SocketWrapper::accept(unsigned int timeout) throw(SocketException)
{
   return getSocket()->accept(timeout);
}

void SocketWrapper::connect(SocketAddress* address, unsigned int timeout)
throw(SocketException)
{
   getSocket()->connect(address, timeout);
}

void SocketWrapper::close()
{
   getSocket()->close();
}

int SocketWrapper::receive(char* b, unsigned int offset, unsigned int length)
throw(SocketException)
{
   return getSocket()->receive(b, offset, length);
}

void SocketWrapper::send(char* b, unsigned int offset, unsigned int length)
throw(SocketException)
{
   getSocket()->send(b, offset, length);
}

bool SocketWrapper::isBound()
{
   return getSocket()->isBound();
}

bool SocketWrapper::isListening()
{
   return getSocket()->isListening();
}

bool SocketWrapper::isConnected()
{
   return getSocket()->isConnected();
}

void SocketWrapper::getLocalAddress(SocketAddress* address)
throw(SocketException)
{
   getSocket()->getLocalAddress(address);
}

void SocketWrapper::getRemoteAddress(SocketAddress* address)
throw(SocketException)   
{
   getSocket()->getRemoteAddress(address);
}

InputStream* SocketWrapper::getInputStream()
{
   return getSocket()->getInputStream();
}

OutputStream* SocketWrapper::getOutputStream()
{
   return getSocket()->getOutputStream();
}

void SocketWrapper::setReceiveTimeout(unsigned long long timeout)
{
   getSocket()->setReceiveTimeout(timeout);
}

unsigned long long SocketWrapper::getReceiveTimeout()
{
   return getSocket()->getReceiveTimeout();
}

unsigned int SocketWrapper::getBacklog()
{
   return getSocket()->getBacklog();
}
