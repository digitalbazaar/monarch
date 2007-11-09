/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/WebConnection.h"

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

unsigned long long WebConnection::getBytesRead()
{
   return mConnection->getBytesRead();
}

unsigned long long WebConnection::getBytesWritten()
{
   return mConnection->getBytesWritten();
}

void WebConnection::setReadTimeout(unsigned long timeout)
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

SocketException* WebConnection::getLocalAddress(InternetAddress* address)
{
   return mConnection->getLocalAddress(address);
}

SocketException* WebConnection::getRemoteAddress(InternetAddress* address)
{
   return mConnection->getRemoteAddress(address);
}

void WebConnection::setSocket(Socket* s, bool cleanup)
{
   mConnection->setSocket(s, cleanup);
}

Socket* WebConnection::getSocket()
{
   return mConnection->getSocket();
}

bool WebConnection::mustCleanupSocket()
{
   return mConnection->mustCleanupSocket();
}
