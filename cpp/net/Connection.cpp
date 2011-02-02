/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/Connection.h"

#include "monarch/net/Internet6Address.h"

using namespace monarch::net;
using namespace monarch::rt;

Connection::Connection(Socket* s, bool cleanup) :
   mSocket(s),
   mCleanupSocket(cleanup),
   mLocalAddress(NULL),
   mRemoteAddress(NULL),
   mSecure(false),
   mReadBandwidthThrottler(NULL),
   mWriteBandwidthThrottler(NULL)
{
   // create addresses
   switch(getCommunicationDomain())
   {
      case SocketAddress::IPv4:
         mLocalAddress = new InternetAddress();
         mRemoteAddress = new InternetAddress();
         break;
      case SocketAddress::IPv6:
         mLocalAddress = new Internet6Address();
         mRemoteAddress = new Internet6Address();
         break;
   }

   // get local and remote addresses
   writeLocalAddress(mLocalAddress);
   writeRemoteAddress(mRemoteAddress);

   // create streams
   mInputStream = new ConnectionInputStream(this);
   mOutputStream = new ConnectionOutputStream(this);
}

Connection::~Connection()
{
   // clean up streams
   delete mInputStream;
   delete mOutputStream;

   // clean up addresses
   delete mLocalAddress;
   delete mRemoteAddress;

   // handle socket cleanup
   if(mCleanupSocket)
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
   return !getSocket()->isConnected();
}

inline void Connection::close()
{
   // close socket
   getSocket()->close();
}

inline SocketAddress* Connection::getLocalAddress()
{
   return mLocalAddress;
}

inline SocketAddress* Connection::getRemoteAddress()
{
   return mRemoteAddress;
}

inline bool Connection::writeLocalAddress(SocketAddress* address)
{
   return getSocket()->getLocalAddress(address);
}

inline bool Connection::writeRemoteAddress(SocketAddress* address)
{
   return getSocket()->getRemoteAddress(address);
}

inline SocketAddress::CommunicationDomain Connection::getCommunicationDomain()
{
   return getSocket()->getCommunicationDomain();
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
