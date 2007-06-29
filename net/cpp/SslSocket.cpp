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
using namespace db::rt;

SslSocket::SslSocket(
   SslContext* context, TcpSocket* socket, bool client, bool cleanup) :
   SocketWrapper(socket, cleanup)
{
   // create ssl object
   mSSL = context->createSSL(socket, client);
   
   // allocate bio pair using default sizes (large enough for SSL records)
   BIO_new_bio_pair(&mSSLBio, 0, &mSocketBio, 0);
   
   // assign SSL BIO to SSL
   SSL_set_bio(mSSL, mSSLBio, mSSLBio);
   
   // no ssl session negotiated yet
   mSessionNegotiated = false;
   
   // create input and output streams
   mInputStream = new PeekInputStream(new SocketInputStream(this), true);
   mOutputStream = new SocketOutputStream(this);
}

SslSocket::~SslSocket()
{
   // free SSL object (implicitly frees SSL BIO)
   SSL_free(mSSL);
   
   // free Socket BIO
   BIO_free(mSocketBio);
   
   // destruct input and output streams
   delete mInputStream;
   delete mOutputStream;
}

int SslSocket::tcpRead()
{
   int rval = -1;
   
   // flush the Socket BIO
   if(tcpWrite())
   {
      // determine how many bytes are required from the Socket BIO
      size_t length = BIO_ctrl_get_read_request(mSocketBio);
      if(length > 0 && !Thread::interrupted(false))
      {
         // read from the underlying socket
         InputStream* is = mSocket->getInputStream();
         char b[length];
         int numBytes = 0;
         while(length > 0 && (numBytes = is->read(b, length)) != -1)
         {
            // write to Socket BIO
            BIO_write(mSocketBio, b, numBytes);
            
            // decrement remaining bytes to read
            length -= numBytes;
            
            // update bytes read
            rval = (rval == -1) ? numBytes : rval + numBytes;
         }
      }
   }
   
   return rval;
}

bool SslSocket::tcpWrite()
{
   bool rval = true;
   
   // determine how many bytes can be read from the Socket BIO
   size_t length = BIO_ctrl_pending(mSocketBio);
   if(length > 0)
   {
      // read from the Socket BIO
      char b[length];
      int numBytes = 0;
      while(length > 0 && !Thread::interrupted(false) &&
            (numBytes = BIO_read(mSocketBio, b, length)) != -1)
      {
         // write to underlying socket
         if(rval = mSocket->getOutputStream()->write(b, numBytes))
         {
            // decrement remaining bytes to write
            length -= numBytes;
         }
      }
   }
   
   return rval && !Thread::interrupted(false);
}

bool SslSocket::performHandshake()
{
   Exception* exception = NULL;
   
   // do SSL_do_handshake()
   int ret = 0;
   while(exception == NULL && !Thread::interrupted(false) &&
         (ret = SSL_do_handshake(mSSL)) <= 0)
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
            exception = new SocketException(
               "Could not perform SSL handshake! Socket closed.");
            break;
         case SSL_ERROR_WANT_READ:
            // more data is required from the socket
            if(tcpRead() == -1)
            {
               exception = new SocketException(
                  "Could not perform SSL handshake! Socket closed.");
            }
            break;
         case SSL_ERROR_WANT_WRITE:
            // data must be flushed to the socket
            if(!tcpWrite())
            {
               exception = Thread::getException();
            }
            break;
         default:
            // an error occurred
            exception = new SocketException(
               "Could not perform SSL handshake!",
               ERR_error_string(ERR_get_error(), NULL));
      }
   }
   
   if(exception == NULL)
   {
      // session negotiated
      mSessionNegotiated = true;
   }
   else
   {
      Thread::setException(exception);
   }
   
   return mSessionNegotiated;
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

bool SslSocket::send(const char* b, unsigned int length)
{
   Exception* exception = NULL;
   
   if(!isConnected())
   {
      exception = new SocketException("Cannot write to unconnected Socket!");
   }
   else
   {
      // perform a handshake as necessary
      if(!mSessionNegotiated)
      {
         if(!performHandshake())
         {
            exception = Thread::getException();
         }
      }
      
      // do SSL_write() (implicit handshake performed as necessary)
      int ret = 0;
      bool closed = false;
      while(exception == NULL && !Thread::interrupted(false) &&
            !closed && (ret <= SSL_write(mSSL, b, length)) <= 0)
      {
         // get the last error
         int error = SSL_get_error(mSSL, ret);
         switch(error)
         {
            case SSL_ERROR_ZERO_RETURN:
               // the connection was shutdown
               exception = new SocketException(
                  "Could not write to Socket! Socket closed.",
                  ERR_error_string(ERR_get_error(), NULL));
               break;
            case SSL_ERROR_WANT_READ:
               // more data is required from the socket
               if(tcpRead() == -1)
               {
                  // the connection was shutdown
                  exception = new SocketException(
                     "Could not write to Socket! Socket closed.",
                     strerror(errno));
               }
               break;
            case SSL_ERROR_WANT_WRITE:
               // data must be flushed to the socket
               if(!tcpWrite())
               {
                  exception = Thread::getException();
               }
               break;
            default:
               // an error occurred
               exception = new SocketException(
                  "Could not write to Socket!",
                  ERR_error_string(ERR_get_error(), NULL));
         }
      }
      
      if(exception != NULL)
      {
         // flush all data to the socket
         if(!tcpWrite())
         {
            exception = Thread::getException();
         }
      }
   }
   
   if(exception != NULL)
   {
      Thread::setException(exception);
   }
   
   return exception == NULL;
}

int SslSocket::receive(char* b, unsigned int length)
{
   int rval = -1;
   
   Exception* exception = NULL;
   
   if(!isConnected())
   {
      exception = new SocketException("Cannot read from unconnected Socket!");
   }
   else
   {
      // perform a handshake as necessary
      if(!mSessionNegotiated)
      {
         if(!performHandshake())
         {
            exception = Thread::getException();
         }
      }
      
      // do SSL_read() (implicit handshake performed as necessary)
      int ret = 0;
      bool closed = false;
      while(exception == NULL && !Thread::interrupted(false) && !closed &&
            (ret = SSL_read(mSSL, b, length)) <= 0)
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
               if(!tcpWrite())
               {
                  exception = Thread::getException();
               }
               break;
            default:
               // an error occurred
               exception = new SocketException(
                  "Could not read from Socket!",
                  ERR_error_string(ERR_get_error(), NULL));
         }
      }
      
      // set number of bytes read
      if(!closed && exception == NULL)
      {
         rval = ret;
      }
   }
   
   if(exception != NULL)
   {
      Thread::setException(exception);
   }
   
   return rval;
}

InputStream* SslSocket::getInputStream()
{
   return mInputStream;
}

OutputStream* SslSocket::getOutputStream()
{
   return mOutputStream;
}
