/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/SocketWrapper.h"

using namespace db::io;
using namespace db::net;

SocketWrapper::SocketWrapper(Socket* socket, bool cleanup)
{
   setSocket(socket, cleanup);
}

SocketWrapper::~SocketWrapper()
{
   // cleanup wrapped socket as appropriate
   if(mCleanupSocket && mSocket != NULL)
   {
      delete mSocket;
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

bool SocketWrapper::mustCleanupSocket()
{
   return mCleanupSocket;
}

bool SocketWrapper::bind(SocketAddress* address)
{
   return getSocket()->bind(address);
}

bool SocketWrapper::listen(unsigned int backlog)
{
   return getSocket()->listen(backlog);
}

Socket* SocketWrapper::accept(unsigned int timeout)
{
   return getSocket()->accept(timeout);
}

bool SocketWrapper::connect(SocketAddress* address, unsigned int timeout)
{
   return getSocket()->connect(address, timeout);
}

bool SocketWrapper::send(const char* b, int length)
{
   return getSocket()->send(b, length);
}

int SocketWrapper::receive(char* b, int length)
{
   return getSocket()->receive(b, length);
}

void SocketWrapper::close()
{
   getSocket()->close();
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

bool SocketWrapper::getLocalAddress(SocketAddress* address)
{
   return getSocket()->getLocalAddress(address);
}

bool SocketWrapper::getRemoteAddress(SocketAddress* address)
{
   return getSocket()->getRemoteAddress(address);
}

InputStream* SocketWrapper::getInputStream()
{
   return getSocket()->getInputStream();
}

OutputStream* SocketWrapper::getOutputStream()
{
   return getSocket()->getOutputStream();
}

void SocketWrapper::setSendTimeout(unsigned long timeout)
{
   getSocket()->setSendTimeout(timeout);
}

unsigned long SocketWrapper::getSendTimeout()
{
   return getSocket()->getSendTimeout();
}

void SocketWrapper::setReceiveTimeout(unsigned long timeout)
{
   getSocket()->setReceiveTimeout(timeout);
}

unsigned long SocketWrapper::getReceiveTimeout()
{
   return getSocket()->getReceiveTimeout();
}

unsigned int SocketWrapper::getBacklog()
{
   return getSocket()->getBacklog();
}
