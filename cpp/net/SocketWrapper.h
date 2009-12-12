/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_SocketWrapper_H
#define db_net_SocketWrapper_H

#include "monarch/net/Socket.h"

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
class SocketWrapper : public Socket
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
    * Returns true if the wrapped Socket must be cleaned up by this
    * wrapper when this wrapper is destructed, false if not.
    *
    * @return true if the wrapped Socket must be cleaned up by this
    *         wrapper when this wrapper is destructed, false if not.
    */
   virtual bool mustCleanupSocket();

   /**
    * Binds this Socket to a SocketAddress.
    *
    * @param address the address to bind to.
    *
    * @return true if bound, false if an exception occurred.
    */
   virtual bool bind(SocketAddress* address);

   /**
    * Causes this Socket to start listening for incoming connections.
    *
    * @param backlog the number of connections to keep backlogged.
    *
    * @return true if listening, false if an exception occurred.
    */
   virtual bool listen(unsigned int backlog = 50);

   /**
    * Accepts a connection to this Socket. This method will block until a
    * connection is made to this Socket.
    *
    * The returned Socket will wrap the file descriptor that is used to
    * communicated with the connected socket.
    *
    * @param timeout the timeout, in seconds, 0 for no timeout.
    *
    * @return an allocated Socket to use to communicate with the connected
    *         socket or NULL if an error occurs.
    */
   virtual Socket* accept(unsigned int timeout);

   /**
    * Connects this Socket to the given address.
    *
    * @param address the address to connect to.
    * @param timeout the timeout, in seconds, 0 for no timeout.
    *
    * @return true if connected, false if an exception occurred.
    */
   virtual bool connect(SocketAddress* address, unsigned int timeout = 30);

   /**
    * Writes raw data to this Socket. This method will block until all of
    * the data has been written.
    *
    * Note: This method is *not* preferred. Use getOutputStream() to obtain the
    * output stream for this Socket.
    *
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    *
    * @return true if the data was sent, false if an exception occurred.
    */
   virtual bool send(const char* b, int length);

   /**
    * Reads raw data from this Socket. This method will block until at least
    * one byte can be read or until the end of the stream is reached (the
    * Socket has closed). A value of 0 will be returned if the end of the
    * stream has been reached, otherwise the number of bytes read will be
    * returned.
    *
    * Note: This method is *not* preferred. Use getInputStream() to obtain the
    * input stream for this Socket.
    *
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    *
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream (the Socket has closed) has been reached or -1 if an error
    *         occurred.
    */
   virtual int receive(char* b, int length);

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
    *
    * @return true if the address was populated, false if an exception occurred.
    */
   virtual bool getLocalAddress(SocketAddress* address);

   /**
    * Gets the remote SocketAddress for this Socket.
    *
    * @param address the SocketAddress to populate.
    *
    * @return true if the address was populated, false if an exception occurred.
    */
   virtual bool getRemoteAddress(SocketAddress* address);

   /**
    * Gets the InputStream for reading from this Socket.
    *
    * @return the InputStream for reading from this Socket.
    */
   virtual monarch::io::InputStream* getInputStream();

   /**
    * Gets the OutputStream for writing to this Socket.
    *
    * @return the OutputStream for writing to this Socket.
    */
   virtual monarch::io::OutputStream* getOutputStream();

   /**
    * Sets the send timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to send data.
    *
    * @param timeout the send timeout in milliseconds.
    */
   virtual void setSendTimeout(uint32_t timeout);

   /**
    * Gets the send timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to send data.
    *
    * @return the send timeout in milliseconds.
    */
   virtual uint32_t getSendTimeout();

   /**
    * Sets the receive timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to receive data.
    *
    * @param timeout the receive timeout in milliseconds.
    */
   virtual void setReceiveTimeout(uint32_t timeout);

   /**
    * Gets the receive timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to receive data.
    *
    * @return the receive timeout in milliseconds.
    */
   virtual uint32_t getReceiveTimeout();

   /**
    * Gets the number of Socket connections that can be kept backlogged while
    * listening.
    *
    * @return the number of Socket connections that can be kept backlogged
    *         while listening.
    */
   virtual unsigned int getBacklog();

   /**
    * Gets the file descriptor for this Socket.
    *
    * @return the file descriptor for this Socket.
    */
   virtual int getFileDescriptor();

   /**
    * Gets the communication domain for this Socket, i.e. IPv4, IPv6.
    *
    * @return the communication domain for this Socket.
    */
   virtual SocketAddress::CommunicationDomain getCommunicationDomain();

   /**
    * Sets whether or not this Socket should not block when sending. If true,
    * then its send and OutputStream will return Exceptions when they would
    * block. An exception detail of "wouldBlock" will be set to true and
    * the number of bytes actually sent will be set to "written".
    *
    * @param on true to activate non-blocking send, false not to.
    */
   virtual void setSendNonBlocking(bool on);

   /**
    * Gets whether or not this Socket blocks when sending.
    *
    * @return true if this Socket does not block when sending, false if it
    *         does.
    */
   virtual bool isSendNonBlocking();

   /**
    * Sets whether or not this Socket should not block when receiving. If true,
    * then its receive and InputStream will return Exceptions when they would
    * block. An exception detail of "wouldBlock" will be set to true.
    *
    * @param on true to activate non-blocking receive, false not to.
    */
   virtual void setReceiveNonBlocking(bool on);

   /**
    * Gets whether or not this Socket blocks when receiving.
    *
    * @return true if this Socket does not block when receiving, false if it
    *         does.
    */
   virtual bool isReceiveNonBlocking();
};

} // end namespace net
} // end namespace db
#endif
