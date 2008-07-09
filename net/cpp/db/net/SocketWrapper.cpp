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

inline Socket* SocketWrapper::getSocket()
{
   return mSocket;
}

inline bool SocketWrapper::mustCleanupSocket()
{
   return mCleanupSocket;
}

inline bool SocketWrapper::bind(SocketAddress* address)
{
   return getSocket()->bind(address);
}

inline bool SocketWrapper::listen(unsigned int backlog)
{
   return getSocket()->listen(backlog);
}

inline Socket* SocketWrapper::accept(unsigned int timeout)
{
   return getSocket()->accept(timeout);
}

inline bool SocketWrapper::connect(SocketAddress* address, unsigned int timeout)
{
   return getSocket()->connect(address, timeout);
}

inline bool SocketWrapper::send(const char* b, int length)
{
   return getSocket()->send(b, length);
}

inline int SocketWrapper::receive(char* b, int length)
{
   return getSocket()->receive(b, length);
}

inline void SocketWrapper::close()
{
   getSocket()->close();
}

inline bool SocketWrapper::isBound()
{
   return getSocket()->isBound();
}

inline bool SocketWrapper::isListening()
{
   return getSocket()->isListening();
}

inline bool SocketWrapper::isConnected()
{
   return getSocket()->isConnected();
}

inline bool SocketWrapper::getLocalAddress(SocketAddress* address)
{
   return getSocket()->getLocalAddress(address);
}

inline bool SocketWrapper::getRemoteAddress(SocketAddress* address)
{
   return getSocket()->getRemoteAddress(address);
}

inline InputStream* SocketWrapper::getInputStream()
{
   return getSocket()->getInputStream();
}

inline OutputStream* SocketWrapper::getOutputStream()
{
   return getSocket()->getOutputStream();
}

inline void SocketWrapper::setSendTimeout(uint32_t timeout)
{
   getSocket()->setSendTimeout(timeout);
}

inline uint32_t SocketWrapper::getSendTimeout()
{
   return getSocket()->getSendTimeout();
}

inline void SocketWrapper::setReceiveTimeout(uint32_t timeout)
{
   getSocket()->setReceiveTimeout(timeout);
}

inline uint32_t SocketWrapper::getReceiveTimeout()
{
   return getSocket()->getReceiveTimeout();
}

inline unsigned int SocketWrapper::getBacklog()
{
   return getSocket()->getBacklog();
}

inline int SocketWrapper::getFileDescriptor()
{
   return getSocket()->getFileDescriptor();
}
