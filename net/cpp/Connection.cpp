/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Connection.h"

using namespace db::net;

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

ConnectionInputStream* Connection::getInputStream()
{
   return mInputStream;
}

ConnectionOutputStream* Connection::getOutputStream()
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

void Connection::setReadTimeout(unsigned long long timeout)
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

void Connection::getLocalAddress(SocketAddress* address) throw(SocketException)
{
   getSocket()->getLocalAddress(address);
}

void Connection::getRemoteAddress(SocketAddress* address) throw(SocketException)
{
   getSocket()->getRemoteAddress(address);
}

Socket* Connection::getSocket()
{
   return mSocket;
}
