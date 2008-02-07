/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_WebConnection_H
#define db_net_WebConnection_H

#include "db/net/Connection.h"
#include "db/net/InternetAddress.h"

namespace db
{
namespace net
{

// forward declare WebRequest
class WebRequest;

/**
 * A WebConnection is a class that represents an internet connection that
 * uses web requests and web responses to communicate.
 * 
 * @author Dave Longley
 */
class WebConnection
{
protected:
   /**
    * The wrapped connection.
    */
   Connection* mConnection;
   
   /**
    * True to clean up the Connection when destructing, false not to.
    */
   bool mCleanupConnection;
   
   /**
    * Gets the InputStream for reading from the underlying Connection.
    * 
    * @return the InputStream for reading from the underlying Connection.
    */
   virtual ConnectionInputStream* getInputStream();
   
   /**
    * Gets the OutputStream for writing to the underlying Connection.
    * 
    * @return the OutputStream for writing to the underlying Connection.
    */
   virtual ConnectionOutputStream* getOutputStream();
   
public:
   /**
    * Creates a new WebConnection that wraps the passed Connection.
    * 
    * @param c the Connection to wrap.
    * @param cleanup true to clean up the Connection when this WebConnection is
    *                destructed, false to leave it alone. 
    */
   WebConnection(Connection* c, bool cleanup);
   
   /**
    * Destructs this WebConnection.
    */
   virtual ~WebConnection();
   
   /**
    * Creates a new WebRequest.
    * 
    * The caller of this method is responsible for freeing the created request.
    * 
    * @return the new WebRequest.
    */
   virtual WebRequest* createRequest() = 0;
   
   /**
    * Sets a BandwidthThrottler.
    * 
    * @param bt the BandwidthThrottler to use.
    * @param read true to use the throttler for reading, false to use it for
    *             writing.
    */
   virtual void setBandwidthThrottler(BandwidthThrottler* bt, bool read);
   
   /**
    * Gets a BandwidthThrottler used by this WebConnection.
    * 
    * @param read true to get the BandwidthThrottler used for reading, false to
    *        get the BandwidthThrottler used for writing.
    * 
    * @return a BandwidthThrottler used by this WebConnection (may be NULL).
    */
   virtual BandwidthThrottler* getBandwidthThrottler(bool read);
   
   /**
    * Gets the total number of bytes read from this WebConnection so far. This
    * includes any bytes that were skipped but not any bytes that were peeked.
    * 
    * @return the total number of bytes read so far.
    */
   virtual unsigned long long getBytesRead();
   
   /**
    * Gets the number of bytes written to this WebConnection so far.
    * 
    * @return the number of bytes written so far.
    */
   virtual unsigned long long getBytesWritten();
   
   /**
    * Sets the read timeout for this WebConnection. This is the amount of
    * time that must pass while doing a read before timing out. 
    * 
    * @param timeout the read timeout in milliseconds (0 for no timeout).
    */
   virtual void setReadTimeout(unsigned long timeout);
   
   /**
    * Marks this WebConnection as secure or non-secure.
    *
    * @param secure true to mark this WebConnection as secure, false to
    *               mark it as non-secure.
    */
   virtual void setSecure(bool secure);
   
   /**
    * Returns whether or not this WebConnection has been marked as secure.
    * 
    * @return true if this WebConnection has been marked as secure, false
    *         if it has been marked as non-secure.
    */
   virtual bool isSecure();
   
   /**
    * Returns true if this WebConnection is closed, false if it is not.
    * 
    * @return true if this WebConnection is closed, false if not.
    */
   virtual bool isClosed();
   
   /**
    * Closes this WebConnection.
    */
   virtual void close();
   
   /**
    * Gets the local InternetAddress for this WebConnection.
    * 
    * @param address the InternetAddress to populate.
    * 
    * @return true if the address could be obtained, false if a SocketException
    *         occurred.
    */
   virtual bool getLocalAddress(InternetAddress* address);
   
   /**
    * Gets the remote InternetAddress for this WebConnection.
    * 
    * @param address the InternetAddress to populate.
    * 
    * @return true if the address could be obtained, false if a SocketException
    *         occurred.
    */
   virtual bool getRemoteAddress(InternetAddress* address);
   
   /**
    * Sets the Socket used by this WebConnection. This method is generally used
    * to wrap a WebConnection's existing Socket with a SocketWrapper that uses
    * a different data presentation (i.e. TLS/SSL). The procedure for doing
    * this is:
    * 
    * // given a WebConnection "wc"
    * WebConnection wc;
    * SocketWrapper* wrapper = new SocketWrapper(
    *    wc.getSocket(), wc.mustCleanupSocket());
    * wc.setSocket(wrapper, true);
    * 
    * // then, optionally, set connection to secure or non-secure
    * wc.setSecure(true);
    * 
    * @param s the Socket to use.
    * @param cleanup true to clean up the passed socket when this Connection
    *                is destructed, false not to.
    */
   virtual void setSocket(Socket* socket, bool cleanup);
   
   /**
    * Gets the Socket used by this WebConnection.
    * 
    * @return the Socket used by this WebConnection.
    */
   virtual Socket* getSocket();
   
   /**
    * Returns true if this WebConnection must clean up its socket's memory when
    * it is destructed, false if not.
    * 
    * @return true if this WebConnection, upon destruction, must free its
    *         socket's memory when it is destructed, false if not.
    */
   virtual bool mustCleanupSocket();
};

} // end namespace net
} // end namespace db
#endif
