/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/SslSocket.h"

#include "db/io/PeekInputStream.h"
#include "db/net/SocketDefinitions.h"
#include "db/net/SocketInputStream.h"
#include "db/net/SocketOutputStream.h"
#include "db/rt/DynamicObject.h"

#include <openssl/err.h>

using namespace db::io;
using namespace db::net;
using namespace db::rt;

/**
 * Certificate verification callback. Called whenever a handshake is performed
 * to check the certificate's common name against the host or against a
 * provided alternatives list. This method will be called regardless of whether
 * or not peer verification is on.
 * 
 * @param preverifyOk 1 if the current certificate passed, 0 if not.
 * @param ctx the X.509 certificate store context for certificate verification.
 * 
 * @return 0 to stop certificate chain verification immediately and fail the
 *         current handshake (but the connection will only fail if peer
 *         verification is on), 1 to continue -- if 1 is always returned, then
 *         the SSL connection will not be terminated due to any certificate
 *         verification failures, but any errors that were set will be
 *         available in X509_STORE_CTX.
 */
static int verifyCallback(int preverifyOk, X509_STORE_CTX *ctx)
{
   // only check common name for peer certificate, which is at depth level 0
   // and only check if the certificate was properly signed/verified
   int depth = X509_STORE_CTX_get_error_depth(ctx);
   if(depth == 0 && preverifyOk)
   {
      // get associated SSL socket
      SSL* ssl = (SSL*)X509_STORE_CTX_get_ex_data(
         ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
      SslSocket* self = (SslSocket*)SSL_get_ex_data(ssl, 0);
      
      // get subject name
      X509* x509 = X509_STORE_CTX_get_current_cert(ctx);
      X509_NAME* name = X509_get_subject_name(x509);
      
      // find a common name that matches
      bool commonNameFound = false;
      X509_NAME_ENTRY* entry;
      ASN1_STRING* str;
      unsigned char* value;
      int i = -1;
      do
      {
         i = X509_NAME_get_index_by_NID(name, NID_commonName, i);
         if(i != -1)
         {
            entry = X509_NAME_get_entry(name, i);
            str = X509_NAME_ENTRY_get_data(entry);
            if(ASN1_STRING_to_UTF8(&value, str) != -1)
            {
               if(self->verifyCommonName((const char*)value))
               {
                  commonNameFound = true;
               }
               OPENSSL_free(value);
            }
         }
      }
      while(i != -1 && !commonNameFound);
      
      // if common name not found then certificate verification failure
      if(!commonNameFound)
      {
         // the certificate is signed and valid, but is being used
         // for a common name different from what was requested
         X509_STORE_CTX_set_error(ctx, X509_V_ERR_INVALID_PURPOSE);
         preverifyOk = 0;
      }
   }
   
   return preverifyOk;
}

SslSocket::SslSocket(
   SslContext* context, TcpSocket* socket, bool client, bool cleanup) :
   SocketWrapper(socket, cleanup)
{
   // create ssl object
   mSSL = context->createSSL(socket, client);
   
   // associate this socket with the SSL instance
   SSL_set_ex_data(mSSL, 0, this);
   
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
   
   // free verify common names
   for(VerifyCommonNameList::iterator i = mVerifyCommonNames.begin();
       i != mVerifyCommonNames.end(); i++)
   {
      if(*i != NULL)
      {
         free((char*)*i);
      }
   }
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

void SslSocket::addVerifyCommonName(const char* commonName)
{
   // add common name to list
   mVerifyCommonNames.push_back(
      (commonName != NULL) ? strdup(commonName) : NULL);
   
   // set verify callback (retain verify mode) if adding first common name
   if(mVerifyCommonNames.size() == 1)
   {
      SSL_set_verify(mSSL, mSSL->verify_mode, verifyCallback);
   }
}

std::vector<const char*>& SslSocket::getVerifyCommonNames()
{
   return mVerifyCommonNames;
}

bool SslSocket::verifyCommonName(const char* commonName)
{
   bool rval = false;
   
   for(VerifyCommonNameList::iterator i = mVerifyCommonNames.begin();
       !rval && i != mVerifyCommonNames.end(); i++)
   {
      if(strcmp(*i, commonName) == 0)
      {
         // common name matches
         rval = true;
      }
   }
   
   return rval;
}

bool SslSocket::performHandshake()
{
   bool rval = true;
   
   // do SSL_do_handshake()
   int ret = 0;
   while(rval && (ret = SSL_do_handshake(mSSL)) <= 0)
   {
      // get the last error
      int error = SSL_get_error(mSSL, ret);
      switch(error)
      {
         case SSL_ERROR_ZERO_RETURN:
         {
            ExceptionRef e = new Exception(
               "Could not perform SSL handshake. Socket closed.",
               SOCKET_EXCEPTION_TYPE ".SslHandshakeError");
            Exception::setLast(e, false);
            rval = false;
            break;
         }
         case SSL_ERROR_WANT_READ:
         {
            // more data is required from the socket
            ret = tcpRead();
            if(ret <= 0)
            {
               ExceptionRef e = new Exception(
                  "Could not perform SSL handshake. Socket closed.",
                  SOCKET_EXCEPTION_TYPE ".SslHandshakeError");
               Exception::setLast(e, (ret < 0));
               rval = false;
            }
            break;
         }
         case SSL_ERROR_WANT_WRITE:
            // data must be flushed to the socket
            rval = tcpWrite();
            break;
         default:
         {
            // an error occurred
            ExceptionRef e = new Exception(
               "Could not perform SSL handshake.",
               SOCKET_EXCEPTION_TYPE ".SslHandshakeError");
            e->getDetails()["error"] = SslContext::getSslErrorStrings();
            Exception::setLast(e, false);
            rval = false;
            break;
         }
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
      ExceptionRef e = new Exception(
         "Cannot write to unconnected socket.",
         SOCKET_EXCEPTION_TYPE ".WriteError");
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
      while(rval && !closed && (ret <= SSL_write(mSSL, b, length)) <= 0)
      {
         // get the last error
         int error = SSL_get_error(mSSL, ret);
         switch(error)
         {
            case SSL_ERROR_ZERO_RETURN:
            {
               // the connection was shutdown
               ExceptionRef e = new Exception(
                  "Could not write to socket. Socket closed.",
                  SOCKET_EXCEPTION_TYPE ".WriteError");
               e->getDetails()["error"] = SslContext::getSslErrorStrings();
               Exception::setLast(e, false);
               rval = false;
               break;
            }
            case SSL_ERROR_WANT_READ:
               // more data is required from the socket
               ret = tcpRead();
               if(ret <= 0)
               {
                  // the connection was shutdown
                  ExceptionRef e = new Exception(
                     "Could not write to socket. Socket closed.",
                     SOCKET_EXCEPTION_TYPE ".WriteError");
                  e->getDetails()["error"] = strerror(errno);
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
               ExceptionRef e = new Exception(
                  "Could not write to socket.",
                  SOCKET_EXCEPTION_TYPE ".WriteError");
               e->getDetails()["error"] = SslContext::getSslErrorStrings();
               Exception::setLast(e, false);
               rval = false;
               break;
            }
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
      ExceptionRef e = new Exception(
         "Cannot read from unconnected socket.",
         SOCKET_EXCEPTION_TYPE ".ReadError");
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
      while(rval != -1 && !closed && (ret = SSL_read(mSSL, b, length)) <= 0)
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
                  ExceptionRef e = new Exception(
                     "Could not read from socket.",
                     SOCKET_EXCEPTION_TYPE ".ReadError");
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
               ExceptionRef e = new Exception(
                  "Could not read from socket.",
                  SOCKET_EXCEPTION_TYPE ".ReadError");
               e->getDetails()["error"] = SslContext::getSslErrorStrings();
               Exception::setLast(e, false);
               rval = -1;
               break;
            }
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

int SslSocket::tcpRead()
{
   int rval = 0;
   
   // flush the Socket BIO
   if(tcpWrite())
   {
      // determine how many bytes are required from the Socket BIO
      size_t length = BIO_ctrl_get_read_request(mSocketBio);
      if(length > 0)
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
         
         if(numBytes < 0)
         {
            // exception reading from input stream
            rval = -1;
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
   size_t length = BIO_ctrl_pending(mSocketBio);
   if(length > 0)
   {
      // read from the Socket BIO
      char b[length];
      int numBytes = 0;
      while(rval && length > 0 &&
            (numBytes = BIO_read(mSocketBio, b, length)) > 0)
      {
         // write to underlying socket, decrement length left to read
         rval = mSocket->getOutputStream()->write(b, numBytes);
         length -= numBytes;
      }
   }
   
   return rval;
}
