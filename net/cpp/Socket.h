/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Socket_H
#define Socket_H

#include "InputStream.h"
#include "OutputStream.h"
#include "SocketDefinitions.h"
#include "SocketAddress.h"
#include "SocketException.h"
#include "SocketTimeoutException.h"

namespace db
{
namespace net
{

/**
 * A Socket is an interface for an end point for communication.
 * 
 * @author Dave Longley
 */
class Socket
{
public:
   /**
    * Creates a new Socket.
    */
   Socket() {};
   
   /**
    * Destructs this Socket.
    */
   virtual ~Socket() {};
   
   /**
    * Binds this Socket to a SocketAddress.
    * 
    * @param address the address to bind to.
    * 
    * @exception SocketException thrown if the address could not be bound.
    */
   virtual void bind(SocketAddress* address) throw(SocketException) = 0;
   
   /**
    * Causes this Socket to start listening for incoming connections.
    * 
    * @param backlog the number of connections to keep backlogged.
    * 
    * @exception SocketException thrown if the address could not be bound.
    */
   virtual void listen(unsigned int backlog = 50) throw(SocketException) = 0;
   
   /**
    * Accepts a connection to this Socket. This method will block until a
    * connection is made to this Socket. If this socket is 
    * 
    * The passed socket will be initialized to the file descriptor that points
    * to the socket that can be used to communicate with the connected socket.
    * 
    * @param socket the socket to use to communicate with the connected socket.
    * @param timeout the timeout, in seconds, 0 for no timeout.
    * 
    * @return a new Socket that controls the socket that can be used to
    *         communicate with the connected socket.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual Socket* accept(unsigned int timeout) throw(SocketException) = 0;
   
   /**
    * Connects this Socket to the given address.
    * 
    * @param address the address to connect to.
    * @param timeout the timeout, in seconds, 0 for no timeout.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual void connect(SocketAddress* address, unsigned int timeout = 30)
   throw(SocketException) = 0;
   
   /**
    * Closes this Socket. This will be done automatically when the Socket is
    * destructed.
    */
   virtual void close() = 0;
   
   /**
    * Reads raw data from this Socket. This method will block until at least
    * one byte can be read or until the end of the stream is reached (the
    * Socket has closed). A value of -1 will be returned if the end of the
    * stream has been reached, otherwise the number of bytes read will be
    * returned.
    * 
    * Note: This method is *not* preferred. Use getInputStream() to obtain the
    * input stream for this Socket.
    * 
    * @param b the array of bytes to fill.
    * @param offset the offset at which to start filling the array.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream (the Socket has closed) has been reached.
    * 
    * @exception SocketException thrown if a socket error occurs. 
    */
   virtual int receive(char* b, unsigned int offset, unsigned int length)
   throw(SocketException) = 0;
   
   /**
    * Writes raw data to this Socket. This method will block until all of
    * the data has been written.
    * 
    * Note: This method is *not* preferred. Use getOutputStream() to obtain the
    * output stream for this Socket.
    * 
    * @param b the array of bytes to write.
    * @param offset the offset at which to start reading from the array.
    * @param length the number of bytes to write to the stream.
    * 
    * @exception SocketException thrown if a socket error occurs. 
    */
   virtual void send(char* b, unsigned int offset, unsigned int length)
   throw(SocketException) = 0;
   
   /**
    * Returns true if this Socket is bound, false if not.
    * 
    * @return true if this Socket is bound, false if not.
    */
   virtual bool isBound() = 0;
   
   /**
    * Returns true if this Socket is listening, false if not.
    * 
    * @return true if this Socket is listening, false if not.
    */
   virtual bool isListening() = 0;
   
   /**
    * Returns true if this Socket is connected, false if not.
    * 
    * @return true if this Socket is connected, false if not.
    */
   virtual bool isConnected() = 0;
   
   /**
    * Gets the local SocketAddress for this Socket.
    * 
    * @param address the SocketAddress to populate.
    */
   virtual void getLocalAddress(SocketAddress* address)
   throw(SocketException) = 0;
   
   /**
    * Gets the remote SocketAddress for this Socket.
    * 
    * @param address the SocketAddress to populate.
    */
   virtual void getRemoteAddress(SocketAddress* address)
   throw(SocketException) = 0;   
   
   /**
    * Gets the InputStream for reading from this Socket.
    * 
    * @return the InputStream for reading from this Socket.
    */
   virtual db::io::InputStream* getInputStream() = 0;
   
   /**
    * Gets the OutputStream for writing to this Socket.
    * 
    * @return the OutputStream for writing to this Socket.
    */
   virtual db::io::OutputStream* getOutputStream() = 0;
   
   /**
    * Sets the receive timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to receive data.
    * 
    * @param timeout the receive timeout in milliseconds.
    */
   virtual void setReceiveTimeout(unsigned long long timeout) = 0;
   
   /**
    * Gets the receive timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to receive data.
    * 
    * @return the receive timeout in milliseconds.
    */
   virtual unsigned long long getReceiveTimeout() = 0;
   
   /**
    * Gets the number of Socket connections that can be kept backlogged while
    * listening.
    * 
    * @return the number of Socket connections that can be kept backlogged
    *         while listening.
    */
   virtual unsigned int getBacklog() = 0;
};

} // end namespace net
} // end namespace db
#endif
