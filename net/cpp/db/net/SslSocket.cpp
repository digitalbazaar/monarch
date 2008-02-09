/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/SslSocket.h"
#include "db/io/PeekInputStream.h"
#include "db/net/SocketInputStream.h"
#include "db/net/SocketOutputStream.h"
#include "db/rt/Thread.h"

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
   int rval = 0;
   
   // flush the Socket BIO
   if(tcpWrite())
   {
      // determine how many bytes are required from the Socket BIO
      Thread* t = Thread::currentThread();
      size_t length = BIO_ctrl_get_read_request(mSocketBio);
      if(length > 0 && !t->isInterrupted())
      {
         // read from the underlying socket
         InputStream* is = mSocket->getInputStream();
         char b[length];
         int numBytes = 0;
         while(length > 0 && (numBytes = is->read(b, length)) > 0)
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
   else
   {
      // exception during tcpWrite()
      rval = -1;
   }
   
   return rval;
}

bool SslSocket::tcpWrite()
{
   bool rval = true;
   
   // determine how many bytes can be read from the Socket BIO
   Thread* t = Thread::currentThread();
   size_t length = BIO_ctrl_pending(mSocketBio);
   if(length > 0)
   {
      // read from the Socket BIO
      char b[length];
      int numBytes = 0;
      while(rval && length > 0 && !t->isInterrupted() &&
            (numBytes = BIO_read(mSocketBio, b, length)) > 0)
      {
         // write to underlying socket, decrement length left to read
         rval = mSocket->getOutputStream()->write(b, numBytes);
         length -= numBytes;
      }
   }
   
   return rval && !t->isInterrupted();
}

void SslSocket::setSession(SslSession* session)
{
   if(session != NULL && (*session) != NULL && (*session)->session != NULL)
   {
      SSL_set_session(mSSL, (*session)->session);
   }
}

SslSession SslSocket::getSession()
{
   // get SSL_SESSION and increment reference count
   SSL_SESSION* s = SSL_get1_session(mSSL);
   SslSession rval(new SslSessionImpl(s));
   return rval;
}

bool SslSocket::performHandshake()
{
   bool rval = true;
   
   // do SSL_do_handshake()
   int ret = 0;
   Thread* t = Thread::currentThread();
   while(rval && !t->isInterrupted() &&
         (ret = SSL_do_handshake(mSSL)) <= 0)
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
            {
               ExceptionRef e = new SocketException(
                  "Could not perform SSL handshake! Socket closed.");
               Exception::setLast(e, false);
               rval = false;
            }
            break;
         case SSL_ERROR_WANT_READ:
            {
               // more data is required from the socket
               ret = tcpRead();
               if(ret <= 0)
               {
                  ExceptionRef e = new SocketException(
                     "Could not perform SSL handshake! Socket closed.");
                  Exception::setLast(e, (ret < 0));
                  rval = false;
               }
            }
            break;
         case SSL_ERROR_WANT_WRITE:
            // data must be flushed to the socket
            rval = tcpWrite();
            break;
         default:
            {
               // an error occurred
               ExceptionRef e = new SocketException(
                  "Could not perform SSL handshake!",
                  ERR_error_string(ERR_get_error(), NULL));
               Exception::setLast(e, false);
               rval = false;
            }
            break;
      }
   }
   
   if(rval)
   {
      // session negotiated
      mSessionNegotiated = true;
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

bool SslSocket::send(const char* b, int length)
{
   bool rval = true;
   
   if(!isConnected())
   {
      ExceptionRef e = new SocketException(
         "Cannot write to unconnected socket!");
      Exception::setLast(e, false);
      rval = false;
   }
   else
   {
      // perform a handshake as necessary
      if(!mSessionNegotiated)
      {
         rval = performHandshake();
      }
      
      // do SSL_write() (implicit handshake performed as necessary)
      int ret = 0;
      bool closed = false;
      Thread* t = Thread::currentThread();
      while(rval && !t->isInterrupted() &&
            !closed && (ret <= SSL_write(mSSL, b, length)) <= 0)
      {
         // get the last error
         int error = SSL_get_error(mSSL, ret);
         switch(error)
         {
            case SSL_ERROR_ZERO_RETURN:
               {
                  // the connection was shutdown
                  ExceptionRef e = new SocketException(
                     "Could not write to socket! Socket closed.",
                     ERR_error_string(ERR_get_error(), NULL));
                  Exception::setLast(e, false);
                  rval = false;
               }
               break;
            case SSL_ERROR_WANT_READ:
               // more data is required from the socket
               ret = tcpRead();
               if(ret <= 0)
               {
                  // the connection was shutdown
                  ExceptionRef e = new SocketException(
                     "Could not write to socket! Socket closed.",
                     strerror(errno));
                  Exception::setLast(e, (ret < 0));
                  rval = false;
               }
               break;
            case SSL_ERROR_WANT_WRITE:
               // data must be flushed to the socket
               rval = tcpWrite();
               break;
            default:
               {
                  // an error occurred
                  ExceptionRef e = new SocketException(
                     "Could not write to socket!",
                     ERR_error_string(ERR_get_error(), NULL));
                  Exception::setLast(e, false);
                  rval = false;
               }
               break;
         }
      }
      
      // flush all data to the socket
      rval = rval && tcpWrite();
   }
   
   return rval;
}

int SslSocket::receive(char* b, int length)
{
   int rval = 0;
   
   if(!isConnected())
   {
      ExceptionRef e = new SocketException(
         "Cannot read from unconnected socket!");
      Exception::setLast(e, false);
      rval = -1;
   }
   else
   {
      // perform a handshake as necessary
      if(!mSessionNegotiated)
      {
         if(!performHandshake())
         {
            rval = -1;
         }
      }
      
      // do SSL_read() (implicit handshake performed as necessary)
      int ret = 0;
      bool closed = false;
      Thread* t = Thread::currentThread();
      while(rval != -1 && !t->isInterrupted() && !closed &&
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
               ret = tcpRead();
               if(ret == 0)
               {
                  // the connection was shutdown properly
                  closed = true;
               }
               else if(ret == -1)
               {
                  // error in writing to socket
                  ExceptionRef e = new SocketException(
                     "Could not read from socket!");
                  Exception::setLast(e, true);
                  rval = -1;
               }
               break;
            case SSL_ERROR_WANT_WRITE:
               // data must be flushed to the socket
               if(!tcpWrite())
               {
                  rval = -1;
               }
               break;
            default:
               {
                  // an error occurred
                  ExceptionRef e = new SocketException(
                     "Could not read from socket!",
                     ERR_error_string(ERR_get_error(), NULL));
                  Exception::setLast(e, false);
                  rval = -1;
               }
               break;
         }
      }
      
      // set number of bytes read
      if(rval != -1)
      {
         rval = (closed) ? 0 : ret;
      }
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
