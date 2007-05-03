/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SslSocket.h"
#include "PeekInputStream.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#include <openssl/err.h>

using namespace db::io;
using namespace db::net;

SslSocket::SslSocket(
   SslContext* context, TcpSocket* socket, bool client, bool cleanup) :
   SocketWrapper(socket, cleanup)
{
   // create ssl object
   mSSL = context->createSSL(socket, client);
   
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
   BIO_free(mWriteBio);
   
   // destruct input and output streams
   delete mInputStream;
   delete mOutputStream;
}

int SslSocket::tcpRead() throw(IOException)
{
   int rval = -1;
   
   // determine how many bytes must be read into the SSL read BIO
   size_t length = BIO_ctrl_get_read_request(mReadBio);
   
   // read from the underlying socket
   InputStream* is = mSocket->getInputStream();
   char b[length];
   int numBytes = 0;
   while(length > 0 && (numBytes = is->read(b, 0, length)) != -1)
   {
      // write to SSL read BIO
      BIO_write(mReadBio, b, numBytes);
      
      // decrement remaining bytes to read
      length -= numBytes;
      
      // update bytes read
      rval = (rval == -1) ? numBytes : rval + numBytes;
   }
   
   return rval;
}

void SslSocket::tcpWrite() throw(IOException)
{
   // determine how many bytes must be written from the SSL write BIO
   size_t length = BIO_ctrl_pending(mWriteBio);
   
   // read from the SSL write BIO
   char b[length];
   int numBytes = 0;
   while(length > 0 && (numBytes = BIO_read(mWriteBio, b, length)) != -1)
   {
      // write to underlying socket
      mSocket->getOutputStream()->write(b, 0, numBytes);
      
      // decrement remaining bytes to write
      length -= numBytes;
   }
}

int SslSocket::receive(char* b, int offset, int length)
throw(SocketException)
{
   int rval = -1;
   
   if(!isConnected())
   {
      throw SocketException("Cannot read from unconnected Socket!");
   }
   
   // do SSL_read()
   int ret = 0;
   bool closed = false;
   while(!closed && (ret = SSL_read(mSSL, b + offset, length)) <= 0)
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
            // the connection was shutdown
            closed = true;
            break;
         case SSL_ERROR_WANT_READ:
            // more data is required from the socket
            if(tcpRead() == -1)
            {
               // the connection was shutdown
               closed = true;
            }
            break;
         case SSL_ERROR_WANT_WRITE:
            // data must be flushed to the socket
            tcpWrite();
            break;
         default:
            // an error occurred
            throw SocketException(
               "Could not read from Socket!", ERR_error_string(error, NULL));
      }
   }
   
   // set number of bytes read
   if(!closed)
   {
      rval = ret;
   }
   
   return rval;
}

void SslSocket::send(char* b, int offset, int length)
throw(SocketException)
{
   if(!isConnected())
   {
      throw SocketException("Cannot write to unconnected Socket!");
   }
   
   // do SSL_write()
   int ret = 0;
   while((ret <= SSL_write(mSSL, b + offset, length)) <= 0)
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
            // the connection was shutdown
            throw SocketException(
               "Could not write to Socket! Socket closed.",
               ERR_error_string(error, NULL));
            break;
         case SSL_ERROR_WANT_READ:
            // more data is required from the socket
            if(tcpRead() == -1)
            {
               // the connection was shutdown
               throw SocketException(
                  "Could not write to Socket! Socket closed.", strerror(errno));
            }
            break;
         case SSL_ERROR_WANT_WRITE:
            // data must be flushed to the socket
            tcpWrite();
            break;
         default:
            // an error occurred
            throw SocketException(
               "Could not write to Socket!",
               ERR_error_string(error, NULL));
      }
   }
}

InputStream* SslSocket::getInputStream()
{
   return mInputStream;
}

OutputStream* SslSocket::getOutputStream()
{
   return mOutputStream;
}
