/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Socket_H
#define Socket_H

#include "Object.h"
#include "IOException.h"
#include "SocketAddress.h"
#include "SocketException.h"
#include "SocketTimeoutException.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

namespace db
{
namespace net
{

/**
 * A Socket is an end point for communication. This is the base class for
 * other types of Sockets that have specific implementations.
 * 
 * @author Dave Longley
 */
class Socket : public virtual db::rt::Object
{
protected:
   /**
    * A file descriptor for the Socket. 
    */
   int mFileDescriptor;
   
   /**
    * The stream for reading from the Socket.
    */
   SocketInputStream mInputStream;
   
   /**
    * The stream for writing to the Socket.
    */
   SocketOutputStream mOutputStream;
   
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
    * Initializes this Socket by acquiring a file descriptor for it. This
    * method must be called before trying to use this Socket.
    * 
    * @exception SocketException thrown if the Socket could not be initialized.
    */
   virtual void initialize() throw(SocketException) = 0;

public:
   /**
    * Creates a new Socket.
    */
   Socket();
   
   /**
    * Destructs this Socket.
    */
   virtual ~Socket();
   
   /**
    * Binds this Socket to a SocketAddress.
    * 
    * @param address the address to bind to.
    * 
    * @exception SocketException thrown if the address could not be bound.
    */
   virtual void bind(SocketAddress* address) throw(SocketException) = 0;
   
   /**
    * Listens for a connection to this Socket and accepts it. This method
    * will block until a connection is made to this Socket.
    * 
    * The passed socket will be initialized to the file descriptor that points
    * to the socket that can be used to communicate with the connected socket.
    * 
    * @param socket the socket to use to communicate with the connected socket.
    * @param timeout the timeout, in seconds, 0 for no timeout.
    * 
    * @exception SocketException thrown if a socket error occurs.
    * @exception SocketTimeoutException thrown if the timeout is reached
    *            before a connection was made.
    */
   virtual void accept(Socket* socket, unsigned int timeout)
   throw(SocketException, SocketTimeoutException) = 0;
   
   /**
    * Connects this Socket to the given address.
    * 
    * @param address the address to connect to.
    * @param timeout the timeout, in seconds, 0 for no timeout.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual void connect(SocketAddress* address, unsigned int timeout)
   throw(SocketException) = 0;
   
   /**
    * Closes this Socket. This will be done automatically when the Socket is
    * destructed.
    */
   virtual void close();
   
   /**
    * Gets the SocketInputStream for reading from this Socket.
    * 
    * @return the SocketInputStream for reading from this Socket.
    */
   virtual SocketInputStream& getInputStream();
   
   /**
    * Gets the SocketOutputStream for writing to this Socket.
    * 
    * @return the SocketOutputStream for writing to this Socket.
    */
   virtual SocketOutputStream& getOutputStream();
   
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
    * @return the number of bytes read from the stream of -1 if the end of the
    *         stream (the Socket has closed) has been reached.
    * 
    * @exception IOException thrown if an IO error occurs. 
    */
   virtual int receive(char* b, int offset, int length)
   throw(db::io::IOException);
   
   /**
    * Writes raw data to this Socket.
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
   virtual void send(char* b, int offset, int length)
   throw (db::io::IOException);
};

} // end namespace net
} // end namespace db
#endif
