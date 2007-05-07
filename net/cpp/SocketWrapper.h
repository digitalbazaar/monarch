/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SocketWrapper_H
#define SocketWrapper_H

#include "Socket.h"

namespace db
{
namespace net
{

/**
 * A SocketWrapper wraps an existing Socket and may add functionality to
 * manipulate that underlying Socket or how it is used in some fashion.
 * 
 * @author Dave Longley
 */
class SocketWrapper : public virtual db::rt::Object, public Socket
{
protected:
   /**
    * The wrapped Socket.
    */
   Socket* mSocket;
   
   /**
    * Set to true to clean up the memory for the wrapped Socket upon
    * destruction, false otherwise.
    */
   bool mCleanupSocket;
   
public:
   /**
    * Creates a new SocketWrapper around the passed Socket.
    * 
    * @param socket the Socket to wrap.
    * @param cleanup true to reclaim the memory used for the wrapped Socket
    *                upon destruction, false to do nothing.
    */
   SocketWrapper(Socket* socket, bool cleanup = false);
   
   /**
    * Destructs this SocketWrapper.
    */
   virtual ~SocketWrapper();
   
   /**
    * Sets the wrapped Socket.
    *
    * @param socket the Socket to wrap.
    * @param cleanup true to reclaim the memory used for the wrapped Socket
    *                upon destruction, false to do nothing.
    */
   virtual void setSocket(Socket* socket, bool cleanup = false);
   
   /**
    * Gets the wrapped Socket.
    * 
    * @return the wrapped Socket.
    */
   virtual Socket* getSocket();
   
   /**
    * Binds this Socket to a SocketAddress.
    * 
    * @param address the address to bind to.
    * 
    * @exception SocketException thrown if the address could not be bound.
    */
   virtual void bind(SocketAddress* address) throw(SocketException);
   
   /**
    * Causes this Socket to start listening for incoming connections.
    * 
    * @param backlog the number of connections to keep backlogged.
    * 
    * @exception SocketException thrown if the address could not be bound.
    */
   virtual void listen(unsigned int backlog = 50) throw(SocketException);
   
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
   virtual Socket* accept(unsigned int timeout) throw(SocketException);
   
   /**
    * Connects this Socket to the given address.
    * 
    * @param address the address to connect to.
    * @param timeout the timeout, in seconds, 0 for no timeout.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual void connect(SocketAddress* address, unsigned int timeout = 30)
   throw(SocketException);
   
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
    * @exception IOException thrown if an IO error occurs. 
    */
   virtual void send(const char* b, unsigned int offset, unsigned int length)
   throw(db::io::IOException);
   
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
    * @exception IOException thrown if an IO error occurs. 
    */
   virtual int receive(char* b, unsigned int offset, unsigned int length)
   throw(db::io::IOException);
   
   /**
    * Closes this Socket. This will be done automatically when the Socket is
    * destructed.
    */
   virtual void close();
   
   /**
    * Returns true if this Socket is bound, false if not.
    * 
    * @return true if this Socket is bound, false if not.
    */
   virtual bool isBound();
   
   /**
    * Returns true if this Socket is listening, false if not.
    * 
    * @return true if this Socket is listening, false if not.
    */
   virtual bool isListening();
   
   /**
    * Returns true if this Socket is connected, false if not.
    * 
    * @return true if this Socket is connected, false if not.
    */
   virtual bool isConnected();
   
   /**
    * Gets the local SocketAddress for this Socket.
    * 
    * @param address the SocketAddress to populate.
    */
   virtual void getLocalAddress(SocketAddress* address) throw(SocketException);
   
   /**
    * Gets the remote SocketAddress for this Socket.
    * 
    * @param address the SocketAddress to populate.
    */
   virtual void getRemoteAddress(SocketAddress* address) throw(SocketException);   
   
   /**
    * Gets the InputStream for reading from this Socket.
    * 
    * @return the InputStream for reading from this Socket.
    */
   virtual db::io::InputStream* getInputStream();
   
   /**
    * Gets the OutputStream for writing to this Socket.
    * 
    * @return the OutputStream for writing to this Socket.
    */
   virtual db::io::OutputStream* getOutputStream();
   
   /**
    * Sets the receive timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to receive data.
    * 
    * @param timeout the receive timeout in milliseconds.
    */
   virtual void setReceiveTimeout(unsigned long long timeout);
   
   /**
    * Gets the receive timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to receive data.
    * 
    * @return the receive timeout in milliseconds.
    */
   virtual unsigned long long getReceiveTimeout();
   
   /**
    * Gets the number of Socket connections that can be kept backlogged while
    * listening.
    * 
    * @return the number of Socket connections that can be kept backlogged
    *         while listening.
    */
   virtual unsigned int getBacklog();
};

} // end namespace net
} // end namespace db
#endif
