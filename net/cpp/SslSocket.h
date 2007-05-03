/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SslSocket_H
#define SslSocket_H

#include "TcpSocket.h"
#include "SocketWrapper.h"
#include "SslContext.h"

#include <openssl/ssl.h>

namespace db
{
namespace net
{

/**
 * An SslSocket is a Socket that uses the TCP/IP protocol and the Secure
 * Sockets Layer (SSL v2/v3) and Transport Layer Security (TLS v1).
 * 
 * @author Dave Longley
 */
class SslSocket : public SocketWrapper
{
protected:
   /**
    * The SSL object for this socket.
    */
   SSL* mSSL;
   
   /**
    * A BIO (Basic Input/Output) for SSL data to be read from once it has
    * been read from the socket.
    */
   BIO* mReadBio;
   
   /**
    * A BIO (Basic Input/Output) for SSL data to be written to before it is
    * sent out to the socket.
    */
   BIO* mWriteBio;
   
   /**
    * The stream for reading from the Socket.
    */
   db::io::InputStream* mInputStream;
   
   /**
    * The stream for writing to the Socket.
    */
   db::io::OutputStream* mOutputStream;   
   
public:
   /**
    * Creates a new SslSocket that wraps the passed TcpSocket.
    * 
    * @param context the SslContext underwhich to create this socket.
    * @param socket the TcpSocket to wrap.
    * @param client true if the TcpSocket is a client socket, false if it
    *               is a server socket.
    * @param cleanup true to reclaim the memory used for the wrapped Socket
    *                upon destruction, false to do nothing.
    */
   SslSocket(
      SslContext* context,
      TcpSocket* socket, bool client, bool cleanup = false);
   
   /**
    * Destructs this SslSocket.
    */
   virtual ~SslSocket();
   
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
   virtual int receive(char* b, int offset, int length) throw(SocketException);
   
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
    * @exception SocketException thrown if a socket error occurs. 
    */
   virtual void send(char* b, int offset, int length) throw(SocketException);
   
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
};

} // end namespace net
} // end namespace db
#endif
