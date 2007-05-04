/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SslSocket.h"
#include "PeekInputStream.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#include <openssl/err.h>

// TEMPCODE: remove this, it's here for testing
#include <iostream>
using namespace std;

using namespace db::io;
using namespace db::net;

SslSocket::SslSocket(
   SslContext* context, TcpSocket* socket, bool client, bool cleanup) :
   SocketWrapper(socket, cleanup)
{
   // create ssl object
   mSSL = context->createSSL(socket, client);
   
   // FIXME: remove this -- it uses the file descriptor directly for a
   // test only (which works)
   //SSL_set_fd(mSSL, socket->getFileDescriptor());
   
   // allocate bio pair using default sizes (large enough for SSL records)
   BIO_new_bio_pair(&mReadBio, 0, &mWriteBio, 0);
   
   // assign bio pair to SSL
   SSL_set_bio(mSSL, mReadBio, mWriteBio);
   
   // create input and output streams
   mInputStream = new PeekInputStream(new SocketInputStream(this), true);
   mOutputStream = new SocketOutputStream(this);
}

SslSocket::~SslSocket()
{
   // free SSL object (implicitly frees read BIO)
   SSL_free(mSSL);
   
   // free write BIO
   //BIO_free(mWriteBio);
   
   // destruct input and output streams
   delete mInputStream;
   delete mOutputStream;
}

int SslSocket::tcpRead() throw(IOException)
{
   int rval = -1;
   
   // determine how many bytes can be written into the SSL read BIO
   size_t length = BIO_get_write_guarantee(mReadBio);
   
   cout << "--tcpRead() " << length << " bytes" << endl;
   
   // read from the underlying socket
   InputStream* is = mSocket->getInputStream();
   char b[length];
   int numBytes = 0;
   while(length > 0 && (numBytes = is->read(b, 0, length)) != -1)
   {
      cout << "--tcpRead() BIO WRITE " << numBytes << " bytes" << endl;
      
      // write to SSL read BIO
      BIO_write(mReadBio, b, numBytes);
      
      // decrement remaining bytes to read
      length -= numBytes;
      
      // update bytes read
      rval = (rval == -1) ? numBytes : rval + numBytes;
   }
   
   cout << "--tcpRead() FINISHED " << rval << " bytes" << endl;
   
   return rval;
}

void SslSocket::tcpWrite() throw(IOException)
{
   // determine how many bytes must be written from the SSL write BIO
   size_t length = BIO_ctrl_pending(mWriteBio);
   
   cout << "--tcpWrite() " << length << " bytes" << endl;
   
   // read from the SSL write BIO
   char b[length];
   int numBytes = 0;
   while(length > 0 && (numBytes = BIO_read(mWriteBio, b, length)) != -1)
   {
      cout << "--tcpWrite() SOCKET WRITE " << numBytes << " bytes" << endl;
      
      // write to underlying socket
      mSocket->getOutputStream()->write(b, 0, numBytes);
      
      // decrement remaining bytes to write
      length -= numBytes;
   }
   
   cout << "--tcpWrite() FINISHED" << endl;
}

void SslSocket::performHandshake() throw(IOException)
{
   cout << "SSL_do_handshake()..." << endl;
   
   // do SSL_do_handshake()
   int ret = 0;
   while((ret = SSL_do_handshake(mSSL)) <= 0)
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
            cout << "SSL_do_handshake() CONNECTION CLOSED" << endl;
            throw SocketException(
               "Could not perform SSL handshake! Socket closed.");
            break;
         case SSL_ERROR_WANT_READ:
            cout << "SSL_do_handshake() WANT READ" << endl;
            // more data is required from the socket
            if(tcpRead() == -1)
            {
               throw SocketException(
                  "Could not perform SSL handshake! Socket closed.");
            }
            break;
         case SSL_ERROR_WANT_WRITE:
            cout << "SSL_do_handshake() WANT WRITE" << endl;
            // data must be flushed to the socket
            tcpWrite();
            break;
         default:
            cout << "SSL_do_handshake() ERROR" << endl;
            // an error occurred
            throw SocketException(
               "Could not perform SSL handshake!",
               ERR_error_string(ERR_get_error(), NULL));
      }
   }
}

void SslSocket::close()
{
   if(isConnected())
   {
      // shutdown SSL
      SSL_shutdown(mSSL);
   }
   
   // close connection
   getSocket()->close();
}

int SslSocket::receive(char* b, unsigned int offset, unsigned int length)
throw(IOException)
{
   int rval = -1;
   
   if(!isConnected())
   {
      throw SocketException("Cannot read from unconnected Socket!");
   }
   
   cout << "SSL_read()..." << endl;
   
   // do SSL_read() (implicit handshake performed as necessary)
   int ret = 0;
   bool closed = false;
   while(!closed && (ret = SSL_read(mSSL, b + offset, length)) <= 0)
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
            cout << "SSL_read() CONNECTION CLOSED" << endl;
            // the connection was shutdown
            closed = true;
            break;
         case SSL_ERROR_WANT_READ:
            cout << "SSL_read() WANT READ" << endl;
            // more data is required from the socket
            if(tcpRead() == -1)
            {
               // the connection was shutdown
               closed = true;
            }
            break;
         case SSL_ERROR_WANT_WRITE:
            cout << "SSL_read() WANT WRITE" << endl;
            // data must be flushed to the socket
            tcpWrite();
            break;
         default:
            cout << "SSL_read() ERROR" << endl;
            // an error occurred
            throw SocketException(
               "Could not read from Socket!",
               ERR_error_string(ERR_get_error(), NULL));
      }
   }
   
   // set number of bytes read
   if(!closed)
   {
      rval = ret;
   }
   
   return rval;
}

void SslSocket::send(char* b, unsigned int offset, unsigned int length)
throw(IOException)
{
   if(!isConnected())
   {
      throw SocketException("Cannot write to unconnected Socket!");
   }
   
   cout << "SSL_write()..." << endl;
   
   // do SSL_write() (implicit handshake performed as necessary)
   int ret = 0;
   bool closed = false;
   while(!closed && (ret <= SSL_write(mSSL, b + offset, length)) <= 0)
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
            cout << "SSL_write() CONNECTION CLOSED" << endl;
            // the connection was shutdown
            throw SocketException(
               "Could not write to Socket! Socket closed.",
               ERR_error_string(ERR_get_error(), NULL));
            break;
         case SSL_ERROR_WANT_READ:
            cout << "SSL_write() WANT READ" << endl;
            // more data is required from the socket
            if(tcpRead() == -1)
            {
               // the connection was shutdown
               throw SocketException(
                  "Could not write to Socket! Socket closed.",
                  strerror(errno));
            }
            break;
         case SSL_ERROR_WANT_WRITE:
            cout << "SSL_write() WANT WRITE" << endl;
            // data must be flushed to the socket
            tcpWrite();
            break;
         default:
            cout << "SSL_write() ERROR" << endl;
            // an error occurred
            throw SocketException(
               "Could not write to Socket!",
               ERR_error_string(ERR_get_error(), NULL));
      }
   }
   
   // flush all data to the socket
   tcpWrite();
}

InputStream* SslSocket::getInputStream()
{
   return mInputStream;
}

OutputStream* SslSocket::getOutputStream()
{
   return mOutputStream;
}
