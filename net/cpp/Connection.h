/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Connection_H
#define Connection_H

#include "Object.h"
#include "Socket.h"
#include "SocketAddress.h"
#include "BandwidthThrottler.h"
#include "ConnectionInputStream.h"
#include "ConnectionOutputStream.h"

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
class Connection : public virtual db::rt::Object
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
    * True if this Connection has been marked secure, false if not.
    */
   bool mSecure;
   
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
   virtual unsigned long long getBytesRead();
   
   /**
    * Gets the number of bytes written to this Connection so far.
    * 
    * @return the number of bytes written so far.
    */
   virtual unsigned long long getBytesWritten();
   
   /**
    * Sets the read timeout for this Connection. This is the amount of
    * time that must pass while doing a read before timing out. 
    * 
    * @param timeout the read timeout in milliseconds (0 for no timeout).
    */
   virtual void setReadTimeout(unsigned long timeout);
   
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
    * Gets the local SocketAddress for this Connection.
    * 
    * @param address the SocketAddress to populate.
    * 
    * @return a SocketException if the address could not be obtained, NULL
    *         otherwise.
    */
   virtual SocketException* getLocalAddress(SocketAddress* address);
   
   /**
    * Gets the remote SocketAddress for this Connection.
    * 
    * @param address the SocketAddress to populate.
    * 
    * @return a SocketException if the address could not be obtained, NULL
    *         otherwise.
    */
   virtual SocketException* getRemoteAddress(SocketAddress* address);
   
   /**
    * Gets the Socket used by this Connection.
    * 
    * @return the Socket used by this Connection.
    */
   virtual Socket* getSocket();
};

} // end namespace net
} // end namespace db
#endif
