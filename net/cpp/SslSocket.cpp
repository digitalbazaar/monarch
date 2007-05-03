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
   // free SSL object
   SSL_free(mSSL);
   
   // free bios
   BIO_free(mReadBio);
   BIO_free(mWriteBio);
   
   // destruct input and output streams
   delete mInputStream;
   delete mOutputStream;
}

int SslSocket::receive(char* b, int offset, int length)
throw(SocketException)
{
   int rval = -1;
   
   if(!isConnected())
   {
      throw SocketException("Cannot read from unconnected Socket!");
   }
   
   // try to do an SSL_read()
   int ret = SSL_read(mSSL, b + offset, length);
   if(ret > 0)
   {
      // operation successful, ret = number of bytes read
      rval = ret;
   }
   else
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
            // the connection was shutdown
            break;
         case SSL_ERROR_WANT_READ:
            // more data is required from the socket
            // FIXME: read more
            break;
         case SSL_ERROR_WANT_WRITE:
            // data must be flushed to the socket
            // FIXME: flush data
            break;
         default:
            // an error occurred
            throw SocketException(
               "Could not read from Socket!", ERR_error_string(error, NULL));
      }
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
   
   // try to do an SSL_write()
   int ret = SSL_write(mSSL, b + offset, length);
   if(ret <= 0)
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
            // the connection was shutdown
            break;
         case SSL_ERROR_WANT_READ:
            // more data is required from the socket
            // FIXME: read more
            break;
         case SSL_ERROR_WANT_WRITE:
            // data must be flushed to the socket
            // FIXME: flush data
            break;
         default:
            // an error occurred
            throw SocketException(
               "Could not write to Socket!", ERR_error_string(error, NULL));
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
