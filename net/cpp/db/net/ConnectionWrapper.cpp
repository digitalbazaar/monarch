/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/ConnectionWrapper.h"

using namespace db::net;

ConnectionWrapper::ConnectionWrapper(Connection* c, bool cleanup)
{
   setConnection(c, cleanup);
}

ConnectionWrapper::~ConnectionWrapper()
{
   // cleanup wrapped connection as appropriate
   if(mConnection != NULL && mCleanupConnection)
   {
      delete mConnection;
   }
}

void ConnectionWrapper::setConnection(Connection* c, bool cleanup)
{
   mConnection = c;
   mCleanupConnection = cleanup;
}

Connection* ConnectionWrapper::getConnection()
{
   return mConnection;
}

bool ConnectionWrapper::mustCleanupConnection()
{
   return mCleanupConnection;
}

void ConnectionWrapper::setBandwidthThrottler(BandwidthThrottler* bt, bool read)
{
   mConnection->setBandwidthThrottler(bt, read);
}

BandwidthThrottler* ConnectionWrapper::getBandwidthThrottler(bool read)
{
   return mConnection->getBandwidthThrottler(read);
}

ConnectionInputStream* ConnectionWrapper::getInputStream()
{
   return mConnection->getInputStream();
}

ConnectionOutputStream* ConnectionWrapper::getOutputStream()
{
   return mConnection->getOutputStream();
}

unsigned long long ConnectionWrapper::getBytesRead()
{
   return mConnection->getBytesRead();
}

unsigned long long ConnectionWrapper::getBytesWritten()
{
   return mConnection->getBytesWritten();
}

void ConnectionWrapper::setReadTimeout(unsigned long timeout)
{
   mConnection->setReadTimeout(timeout);
}

void ConnectionWrapper::setSecure(bool secure)
{
   mConnection->setSecure(secure);
}

bool ConnectionWrapper::isSecure()
{
   return mConnection->isSecure();
}

bool ConnectionWrapper::isClosed()
{
   return mConnection->isClosed();
}

void ConnectionWrapper::close()
{
   mConnection->close();
}

bool ConnectionWrapper::getLocalAddress(InternetAddress* address)
{
   return mConnection->getLocalAddress(address);
}

bool ConnectionWrapper::getRemoteAddress(InternetAddress* address)
{
   return mConnection->getRemoteAddress(address);
}

void ConnectionWrapper::setSocket(Socket* s, bool cleanup)
{
   mConnection->setSocket(s, cleanup);
}

Socket* ConnectionWrapper::getSocket()
{
   return mConnection->getSocket();
}

bool ConnectionWrapper::mustCleanupSocket()
{
   return mConnection->mustCleanupSocket();
}
