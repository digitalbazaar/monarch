/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Connection.h"

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

SocketException* Connection::getLocalAddress(SocketAddress* address)
{
   SocketException* rval = NULL;
   
   if(!getSocket()->getLocalAddress(address))
   {
      rval = (SocketException*)Exception::getLast();
   }
   
   return rval;
}

SocketException* Connection::getRemoteAddress(SocketAddress* address)
{
   SocketException* rval = NULL;
   
   if(!getSocket()->getRemoteAddress(address))
   {
      rval = (SocketException*)Exception::getLast();
   }
   
   return rval;
}

Socket* Connection::getSocket()
{
   return mSocket;
}
