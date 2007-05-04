/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef AbstractSocket_H
#define AbstractSocket_H

#include "Object.h"
#include "Socket.h"
#include "InputStream.h"
#include "OutputStream.h"

namespace db
{
namespace net
{

/**
 * An AbstractSocket provides the basic implementation for a Sock. This is the
 * base class for types of Sockets that have specific protocols and
 * implementations.
 * 
 * @author Dave Longley
 */
class AbstractSocket : public virtual db::rt::Object, public Socket
{
protected:
   /**
    * A file descriptor for the Socket. 
    */
   int mFileDescriptor;
   
   /**
    * True when this Socket is bound, false when not.
    */
   bool mBound;
   
   /**
    * True when this Socket is listening, false when not.
    */
   bool mListening;
   
   /**
    * True when this Socket is connected, false when not.
    */
   bool mConnected;
   
   /**
    * The stream for reading from the Socket.
    */
   db::io::InputStream* mInputStream;
   
   /**
    * The stream for writing to the Socket.
    */
   db::io::OutputStream* mOutputStream;
   
   /**
    * The receive timeout (in milliseconds) for reading from the Socket.
    */
   unsigned long long mReceiveTimeout;
   
   /**
    * The number of Socket connections to keep backlogged while listening.
    */
   unsigned int mBacklog;
   
   /**
    * Populates a sockaddr_in structure using a SocketAddress object.
    * 
    * @param address the SocketAddress object to use.
    * @param addr the sockaddr_in structure to populate.
    */
   virtual void populateAddressStructure(
      SocketAddress* address, sockaddr_in& addr);
   
   /**
    * Creates a Socket with the specified type and protocol and assigns its
    * file descriptor to mFileDescriptor.
    * 
    * @param type the type of Socket to create.
    * @param protocol the protocol for the Socket.
    * 
    * @exception SocketException thrown if the Socket could not be created.
    */
   virtual void create(int type, int protocol) throw(SocketException);
   
   /**
    * Blocks until data is available for receiving or until a connection
    * closes.
    * 
    * @return true if data is available for receiving.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual bool select() throw(SocketException);
   
   /**
    * Initializes this Socket by acquiring a file descriptor for it. This
    * method must be called before trying to use this Socket.
    * 
    * @exception SocketException thrown if the Socket could not be initialized.
    */
   virtual void initialize() throw(SocketException) = 0;
   
   /**
    * Creates a new Socket with the given file descriptor that points to
    * the socket for an accepted connection.
    * 
    * @param fd the file descriptor for the socket.
    * 
    * @return the allocated Socket.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual Socket* createConnectedSocket(unsigned int fd)
   throw(SocketException) = 0;
   
public:
   /**
    * Creates a new AbstractSocket.
    */
   AbstractSocket();
   
   /**
    * Destructs this AbstractSocket.
    */
   virtual ~AbstractSocket();
   
   /**
    * Binds this Socket to a SocketAddress.
    * 
    * @param address the address to bind to.
    * 
    * @exception SocketException thrown if the address could not be bound.
    */
   virtual void bind(SocketAddress* address) throw(SocketException);
   
   /**
    * Binds this Socket to a local port on the local machine's IP address.
    * 
    * This will bind to INADDR_ANY or 0.0.0.0 in IPv4.
    * A port of 0 will bind to a random emphemeral port.
    * 
    * @param port the port to bind to.
    */
   virtual void bind(unsigned short port);
   
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
   virtual void send(char* b, unsigned int offset, unsigned int length)
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
   
   /**
    * Gets the file descriptor for this Socket.
    * 
    * @return the file descriptor for this Socket.
    */
   virtual int getFileDescriptor();
};

} // end namespace net
} // end namespace db
#endif
