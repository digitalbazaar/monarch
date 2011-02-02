/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/ConnectionWrapper.h"

using namespace monarch::net;

ConnectionWrapper::ConnectionWrapper(Connection* c, bool cleanup)
{
   setConnection(c, cleanup);
}

ConnectionWrapper::~ConnectionWrapper()
{
   // cleanup wrapped connection as appropriate
   if(mCleanupConnection)
   {
      delete mConnection;
   }
}

void ConnectionWrapper::setConnection(Connection* c, bool cleanup)
{
   mConnection = c;
   mCleanupConnection = cleanup;
}

inline Connection* ConnectionWrapper::getConnection()
{
   return mConnection;
}

inline bool ConnectionWrapper::mustCleanupConnection()
{
   return mCleanupConnection;
}

inline void ConnectionWrapper::setBandwidthThrottler(
   BandwidthThrottler* bt, bool read)
{
   mConnection->setBandwidthThrottler(bt, read);
}

inline BandwidthThrottler* ConnectionWrapper::getBandwidthThrottler(bool read)
{
   return mConnection->getBandwidthThrottler(read);
}

inline ConnectionInputStream* ConnectionWrapper::getInputStream()
{
   return mConnection->getInputStream();
}

inline ConnectionOutputStream* ConnectionWrapper::getOutputStream()
{
   return mConnection->getOutputStream();
}

inline uint64_t ConnectionWrapper::getBytesRead()
{
   return mConnection->getBytesRead();
}

inline uint64_t ConnectionWrapper::getBytesWritten()
{
   return mConnection->getBytesWritten();
}

inline void ConnectionWrapper::setReadTimeout(uint32_t timeout)
{
   mConnection->setReadTimeout(timeout);
}

inline void ConnectionWrapper::setWriteTimeout(uint32_t timeout)
{
   mConnection->setWriteTimeout(timeout);
}

inline void ConnectionWrapper::setSecure(bool secure)
{
   mConnection->setSecure(secure);
}

inline bool ConnectionWrapper::isSecure()
{
   return mConnection->isSecure();
}

inline bool ConnectionWrapper::isClosed()
{
   return mConnection->isClosed();
}

inline void ConnectionWrapper::close()
{
   mConnection->close();
}

inline SocketAddress* ConnectionWrapper::getLocalAddress()
{
   return mConnection->getLocalAddress();
}

inline SocketAddress* ConnectionWrapper::getRemoteAddress()
{
   return mConnection->getRemoteAddress();
}

inline bool ConnectionWrapper::writeLocalAddress(SocketAddress* address)
{
   return mConnection->writeLocalAddress(address);
}

inline bool ConnectionWrapper::writeRemoteAddress(SocketAddress* address)
{
   return mConnection->writeRemoteAddress(address);
}

inline SocketAddress::CommunicationDomain
   ConnectionWrapper::getCommunicationDomain()
{
   return mConnection->getCommunicationDomain();
}

inline void ConnectionWrapper::setSocket(Socket* s, bool cleanup)
{
   mConnection->setSocket(s, cleanup);
}

inline Socket* ConnectionWrapper::getSocket()
{
   return mConnection->getSocket();
}

inline bool ConnectionWrapper::mustCleanupSocket()
{
   return mConnection->mustCleanupSocket();
}
