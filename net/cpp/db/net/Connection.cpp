/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/Connection.h"

using namespace db::net;
using namespace db::rt;

Connection::Connection(Socket* s, bool cleanup)
{
   mSocket = s;
   mCleanupSocket = cleanup;
   mSecure = false;
   
   // no bandwidth throttlers installed
   mReadBandwidthThrottler = NULL;
   mWriteBandwidthThrottler = NULL;
   
   // create streams
   mInputStream = new ConnectionInputStream(this);
   mOutputStream = new ConnectionOutputStream(this);
}

Connection::~Connection()
{
   // clean up streams
   delete mInputStream;
   delete mOutputStream;
   
   // handle socket cleanup
   if(mSocket != NULL && mCleanupSocket)
   {
      delete mSocket;
   }
}

void Connection::setBandwidthThrottler(BandwidthThrottler* bt, bool read)
{
   // synchronize setting the bandwidth throttler so it is thread safe
   lock();
   {
      if(read)
      {
         mReadBandwidthThrottler = bt;
      }
      else
      {
         mWriteBandwidthThrottler = bt;
      }
   }
   unlock();
}

BandwidthThrottler* Connection::getBandwidthThrottler(bool read)
{
   BandwidthThrottler* rval = NULL;
   
   // synchronize fetching the bandwidth throttler so it is thread safe
   lock();
   {
      rval = read ? mReadBandwidthThrottler : mWriteBandwidthThrottler;
   }
   unlock();
   
   return rval;
}

inline ConnectionInputStream* Connection::getInputStream()
{
   return mInputStream;
}

inline ConnectionOutputStream* Connection::getOutputStream()
{
   return mOutputStream;
}

unsigned long long Connection::getBytesRead()
{
   return mInputStream->getBytesRead();
}

unsigned long long Connection::getBytesWritten()
{
   return mOutputStream->getBytesWritten();
}

void Connection::setReadTimeout(unsigned long timeout)
{
   // set the receive timeout
   getSocket()->setReceiveTimeout(timeout);
}

void Connection::setSecure(bool secure)
{
   mSecure = secure;
}

bool Connection::isSecure()
{
   return mSecure;
}

bool Connection::isClosed()
{
   // check socket
   return !getSocket()->isBound();
}

void Connection::close()
{
   // close socket
   getSocket()->close();
}

bool Connection::getLocalAddress(SocketAddress* address)
{
   return getSocket()->getLocalAddress(address);
}

bool Connection::getRemoteAddress(SocketAddress* address)
{
   return getSocket()->getRemoteAddress(address);
}

void Connection::setSocket(Socket* s, bool cleanup)
{
   mSocket = s;
   mCleanupSocket = cleanup;
}

Socket* Connection::getSocket()
{
   return mSocket;
}

bool Connection::mustCleanupSocket()
{
   return mCleanupSocket;
}
