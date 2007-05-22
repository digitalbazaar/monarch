/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "WebConnection.h"

using namespace db::net;

WebConnection::WebConnection(Connection* c, bool cleanup)
{
   mConnection = c;
   mCleanupConnection = cleanup;
}

WebConnection::~WebConnection()
{
   // handle connection cleanup
   if(mConnection != NULL && mCleanupConnection)
   {
      delete mConnection;
   }
}

ConnectionInputStream* WebConnection::getInputStream()
{
   return mConnection->getInputStream();
}

ConnectionOutputStream* WebConnection::getOutputStream()
{
   return mConnection->getOutputStream();
}

void WebConnection::setBandwidthThrottler(BandwidthThrottler* bt, bool read)
{
   mConnection->setBandwidthThrottler(bt, read);
}

BandwidthThrottler* WebConnection::getBandwidthThrottler(bool read)
{
   return mConnection->getBandwidthThrottler(read);
}

const unsigned long long& WebConnection::getBytesRead()
{
   return mConnection->getBytesRead();
}

const unsigned long long& WebConnection::getBytesWritten()
{
   return mConnection->getBytesWritten();
}

void WebConnection::setReadTimeout(unsigned long long timeout)
{
   mConnection->setReadTimeout(timeout);
}

void WebConnection::setSecure(bool secure)
{
   mConnection->setSecure(secure);
}

bool WebConnection::isSecure()
{
   return mConnection->isSecure();
}

bool WebConnection::isClosed()
{
   return mConnection->isClosed();
}

void WebConnection::close()
{
   mConnection->close();
}

void WebConnection::getLocalAddress(InternetAddress* address)
throw(SocketException)
{
   mConnection->getLocalAddress(address);
}

void WebConnection::getRemoteAddress(InternetAddress* address)
throw(SocketException)
{
   mConnection->getRemoteAddress(address);
}

Socket* WebConnection::getSocket()
{
   return mConnection->getSocket();
}
