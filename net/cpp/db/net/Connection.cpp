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
   mBandwidthThrottlerLock.lockExclusive();
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
   mBandwidthThrottlerLock.unlockExclusive();
}

BandwidthThrottler* Connection::getBandwidthThrottler(bool read)
{
   BandwidthThrottler* rval = NULL;
   
   // synchronize fetching the bandwidth throttler so it is thread safe
   mBandwidthThrottlerLock.lockShared();
   {
      rval = read ? mReadBandwidthThrottler : mWriteBandwidthThrottler;
   }
   mBandwidthThrottlerLock.unlockShared();
   
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

uint64_t Connection::getBytesRead()
{
   return mInputStream->getBytesRead();
}

uint64_t Connection::getBytesWritten()
{
   return mOutputStream->getBytesWritten();
}

inline void Connection::setReadTimeout(uint32_t timeout)
{
   // set the receive timeout
   getSocket()->setReceiveTimeout(timeout);
}

inline void Connection::setWriteTimeout(uint32_t timeout)
{
   // set the send timeout
   getSocket()->setSendTimeout(timeout);
}

inline void Connection::setSecure(bool secure)
{
   mSecure = secure;
}

inline bool Connection::isSecure()
{
   return mSecure;
}

inline bool Connection::isClosed()
{
   // check socket
   return !getSocket()->isBound();
}

inline void Connection::close()
{
   // close socket
   getSocket()->close();
}

inline bool Connection::getLocalAddress(SocketAddress* address)
{
   return getSocket()->getLocalAddress(address);
}

inline bool Connection::getRemoteAddress(SocketAddress* address)
{
   return getSocket()->getRemoteAddress(address);
}

void Connection::setSocket(Socket* s, bool cleanup)
{
   mSocket = s;
   mCleanupSocket = cleanup;
}

inline Socket* Connection::getSocket()
{
   return mSocket;
}

inline bool Connection::mustCleanupSocket()
{
   return mCleanupSocket;
}
