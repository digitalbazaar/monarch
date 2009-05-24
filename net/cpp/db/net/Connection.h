/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_Connection_H
#define db_net_Connection_H

#include "db/rt/SharedLock.h"
#include "db/net/Socket.h"
#include "db/net/SocketAddress.h"
#include "db/net/BandwidthThrottler.h"
#include "db/net/ConnectionInputStream.h"
#include "db/net/ConnectionOutputStream.h"

namespace db
{
namespace net
{

/**
 * A Connection is a class that represents a socket connection between
 * a client and a server. The protocol for data transportation is determined
 * by the Socket used when creating this Connection.
 * 
 * @author Dave Longley
 */
class Connection
{
protected:
   /**
    * The Socket for this Connection.
    */
   Socket* mSocket;
   
   /**
    * True to clean up the Socket when destructing, false not to.
    */
   bool mCleanupSocket;
   
   /**
    * The local SocketAddress for this Connection.
    */
   SocketAddress* mLocalAddress;
   
   /**
    * The remote SocketAddress for this Connection.
    */
   SocketAddress* mRemoteAddress;
   
   /**
    * True if this Connection has been marked secure, false if not.
    */
   bool mSecure;
   
   /**
    * A lock for modifying the bandwidth throttlers.
    */
   db::rt::SharedLock mBandwidthThrottlerLock;
   
   /**
    * The BandwidthThrottler for reading.
    */
   BandwidthThrottler* mReadBandwidthThrottler;
   
   /**
    * The BandwidthThrottler for writing.
    */
   BandwidthThrottler* mWriteBandwidthThrottler;
   
   /**
    * The ConnectionInputStream to read from.
    */
   ConnectionInputStream* mInputStream;
   
   /**
    * The ConnectionOutputStream to write to.
    */
   ConnectionOutputStream* mOutputStream;
   
public:
   /**
    * Creates a new Connection that wraps the passed Socket.
    * 
    * @param s the Socket for this Connection.
    * @param cleanup true to clean up the Socket when this Connection is
    *                destructed, false to leave it alone. 
    */
   Connection(Socket* s, bool cleanup);
   
   /**
    * Destructs this Connection.
    */
   virtual ~Connection();
   
   /**
    * Sets a BandwidthThrottler.
    * 
    * @param bt the BandwidthThrottler to use.
    * @param read true to use the throttler for reading, false to use it for
    *             writing.
    */
   virtual void setBandwidthThrottler(BandwidthThrottler* bt, bool read);
   
   /**
    * Gets a BandwidthThrottler used by this Connection.
    * 
    * @param read true to get the BandwidthThrottler used for reading, false to
    *        get the BandwidthThrottler used for writing.
    * 
    * @return a BandwidthThrottler used by this Connection (may be NULL).
    */
   virtual BandwidthThrottler* getBandwidthThrottler(bool read);
   
   /**
    * Gets the InputStream for reading from this Connection.
    * 
    * @return the InputStream for reading from this Connection.
    */
   virtual ConnectionInputStream* getInputStream();
   
   /**
    * Gets the OutputStream for writing to this Connection.
    * 
    * @return the OutputStream for writing to this Connection.
    */
   virtual ConnectionOutputStream* getOutputStream();
   
   /**
    * Gets the total number of bytes read from this Connection so far. This
    * includes any bytes that were skipped but not any bytes that were peeked.
    * 
    * @return the total number of bytes read so far.
    */
   virtual uint64_t getBytesRead();
   
   /**
    * Gets the number of bytes written to this Connection so far.
    * 
    * @return the number of bytes written so far.
    */
   virtual uint64_t getBytesWritten();
   
   /**
    * Sets the read timeout for this Connection. This is the amount of
    * time that must pass while doing a read before timing out. 
    * 
    * @param timeout the read timeout in milliseconds (0 for no timeout).
    */
   virtual void setReadTimeout(uint32_t timeout);
   
   /**
    * Sets the write timeout for this Connection. This is the amount of
    * time that must pass while doing a write before timing out. 
    * 
    * @param timeout the writetimeout in milliseconds (0 for no timeout).
    */
   virtual void setWriteTimeout(uint32_t timeout);
   
   /**
    * Marks this Connection as secure or non-secure.
    *
    * @param secure true to mark this Connection as secure, false to
    *               mark it as non-secure.
    */
   virtual void setSecure(bool secure);
   
   /**
    * Returns whether or not this Connection has been marked as secure.
    * 
    * @return true if this Connection has been marked as secure, false
    *         if it has been marked as non-secure.
    */
   virtual bool isSecure();
   
   /**
    * Returns true if this Connection is closed, false if it is not.
    * 
    * @return true if this Connection is closed, false if not.
    */
   virtual bool isClosed();
   
   /**
    * Closes this Connection.
    */
   virtual void close();
   
   /**
    * Gets the local address for this Connection. This address can be
    * up-cast to an InternetAddress or Internet6Address based on
    * the communication domain of the returned SocketAddress.
    * 
    * @return the local address.
    */
   virtual SocketAddress* getLocalAddress();
   
   /**
    * Gets the remote address for this Connection. This address can be
    * up-cast to an InternetAddress or Internet6Address based on
    * the communication domain of the returned SocketAddress.
    * 
    * @return the remote address.
    */
   virtual SocketAddress* getRemoteAddress();
   
   /**
    * Writes the local SocketAddress for this Connection into the passed
    * parameter.
    * 
    * @param address the SocketAddress to populate.
    * 
    * @return true if the address could be obtained, false if a SocketException
    *         occurred.
    */
   virtual bool writeLocalAddress(SocketAddress* address);
   
   /**
    * Writes the remote SocketAddress for this Connection into the passed
    * parameter.
    * 
    * @param address the SocketAddress to populate.
    * 
    * @return true if the address could be obtained, false if a SocketException
    *         occurred.
    */
   virtual bool writeRemoteAddress(SocketAddress* address);
   
   /**
    * Gets the communication domain for this Socket, i.e. IPv4, IPv6.
    * 
    * @return the communication domain for this Socket.
    */
   virtual SocketAddress::CommunicationDomain getCommunicationDomain();
   
   /**
    * Sets the Socket used by this Connection. This method is generally used
    * to wrap a Connection's existing Socket with a SocketWrapper that uses
    * a different data presentation (i.e. TLS/SSL). The procedure for doing
    * this is:
    * 
    * // given a Connection "c"
    * Connection c;
    * SocketWrapper* wrapper = new SocketWrapper(
    *    c.getSocket(), c.mustCleanupSocket());
    * c.setSocket(wrapper, true);
    * 
    * // then, optionally, set connection to secure or non-secure
    * c.setSecure(true);
    * 
    * @param s the Socket to use.
    * @param cleanup true to clean up the passed socket when this Connection
    *                is destructed, false not to.
    */
   virtual void setSocket(Socket* socket, bool cleanup);
   
   /**
    * Gets the Socket used by this Connection.
    * 
    * @return the Socket used by this Connection.
    */
   virtual Socket* getSocket();
   
   /**
    * Returns true if this Connection must clean up its socket's memory when
    * it is destructed, false if not.
    * 
    * @return true if this Connection, upon destruction, must free its socket's
    *         memory when it is destructed, false if not.
    */
   virtual bool mustCleanupSocket();
};

} // end namespace net
} // end namespace db
#endif
