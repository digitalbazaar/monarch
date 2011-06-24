/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/SslSocket.h"

#include "monarch/logging/Logging.h"
#include "monarch/io/PeekInputStream.h"
#include "monarch/net/SocketDefinitions.h"
#include "monarch/net/SocketInputStream.h"
#include "monarch/net/SocketOutputStream.h"
#include "monarch/rt/DynamicObject.h"

#include <algorithm>

#include <openssl/err.h>

using namespace std;
using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;

#define TRANSPORT_BUFFER   1024

// FIXME: SSL implementation needs to be abstracted away from SslSocket so
// that it can be used by non-sockets and so the code is cleaner

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
static int _verifyCallback(int preverifyOk, X509_STORE_CTX *ctx)
{
   // only check common name for peer certificate, which is at depth level 0
   // and only check if the certificate was properly signed/verified
   int depth = X509_STORE_CTX_get_error_depth(ctx);
   if(depth == 0 && preverifyOk)
   {
      // get associated SSL socket
      SSL* ssl = (SSL*)X509_STORE_CTX_get_ex_data(
         ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
      SslSocket* self = static_cast<SslSocket*>(SSL_get_ex_data(ssl, 0));

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
   SocketWrapper(socket, cleanup),
   mVirtualHost(NULL)
{
   // create ssl object
   mSSL = context->createSSL(socket, client);

   // associate this socket with the SSL instance
   SSL_set_ex_data(mSSL, 0, this);

   // allocate bio pair using default sizes (large enough for SSL records)
   BIO_new_bio_pair(&mSSLBio, 0, &mSocketBio, 0);

   // assign SSL BIO to SSL
   SSL_set_bio(mSSL, mSSLBio, mSSLBio);

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
       i != mVerifyCommonNames.end(); ++i)
   {
      free((char*)*i);
   }

   free(mVirtualHost);
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
      SSL_set_verify(mSSL, mSSL->verify_mode, _verifyCallback);
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
       !rval && i != mVerifyCommonNames.end(); ++i)
   {
      if(strcmp(*i, commonName) == 0)
      {
         // common name matches
         rval = true;
      }
   }

   // add useful logging output
   if(!rval)
   {
      std::string str;
      for(VerifyCommonNameList::iterator i = mVerifyCommonNames.begin();
          i != mVerifyCommonNames.end(); ++i)
      {
         if(str.length() > 0)
         {
            str.push_back(',');
         }
         str.push_back('\'');
         str.append(*i);
         str.push_back('\'');
      }

      // log error
      MO_CAT_DEBUG(MO_NET_CAT,
         "X.509 certificate verification failure, "
         "no match found for common name '%s', acceptable common names: %s",
         commonName, str.c_str());
   }

   return rval;
}

bool SslSocket::setVirtualHost(const char* name)
{
   return SSL_set_tlsext_host_name(mSSL, name) == 1;
}

/**
 * Reads some raw data from the underlying TCP socket and stores it in the
 * SSL read BIO. This method will block until at least length bytes have been
 * read or until the end of the stream is reached (the Socket has closed).
 *
 * @param b the transport buffer to read data into.
 * @param is the input stream to read from.
 * @param bio the SSL BIO to write to.
 * @param length the number of bytes to read.
 *
 * @return the number of bytes read or 0 if the end of the stream has been
 *         reached (the Socket has closed) or -1 if an exception occurred.
 */
inline static int _tcpRead(char* b, InputStream* is, BIO* bio, int length)
{
   int rval = 0;

   // read from incoming TCP socket, write to SSL BIO
   int numBytes = 0;
   while(length > 0 && (numBytes = is->read(
      b, min(length, TRANSPORT_BUFFER))) > 0)
   {
      BIO_write(bio, b, numBytes);
      length -= numBytes;
      rval += numBytes;
   }

   // exception reading from input stream
   if(numBytes < 0)
   {
      rval = -1;
   }

   return rval;
}

/**
 * Flushes the data from the SSL write BIO to the underlying TCP Socket.
 * This method will block until all of the data has been written.
 *
 * @param b the transport buffer to read data into.
 * @param bio the SSL BIO to read from.
 * @param os the output stream to write to.
 * @param length the number of bytes to write.
 *
 * @return true if the write was successful, false if an exception occurred.
 */
inline static bool _tcpWrite(char* b, BIO* bio, OutputStream* os, int length)
{
   bool rval = true;

   // read from SSL BIO, write out to TCP socket
   int numBytes;
   while(rval && length > 0 && (numBytes = BIO_read(
      bio, b, min(length, TRANSPORT_BUFFER))) > 0)
   {
      rval = os->write(b, numBytes);
      length -= numBytes;
   }

   return rval;
}

/**
 * Handles any SSL-layer communcation between endpoints by flushing any
 * pending SSL data over the TCP connection and by receiving any pending
 * SSL data.
 *
 * @param socket the TCP socket.
 * @param bio the SSL bio.
 *
 * @return the number of bytes read or -1 if an exception occurred.
 */
static int _tcpTransport(Socket* socket, BIO* bio)
{
   int rval = 0;

   // prepare transport vars
   char b[TRANSPORT_BUFFER];
   size_t length;
   InputStream* is = socket->getInputStream();
   OutputStream* os = socket->getOutputStream();

   // flush pending outgoing bytes
   length = BIO_ctrl_pending(bio);
   if(!_tcpWrite(b, bio, os, length))
   {
      rval = -1;
   }
   else
   {
      // receive requested incoming bytes
      length = BIO_ctrl_get_read_request(bio);
      rval = _tcpRead(b, is, bio, length);
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
            // Note: In SSL 3.0/TLS 1.0 this case only occurs when a closure
            // alert has occurred in the protocol, i.e. if the connection has
            // been closed cleanly. It does not necessarily indicate that the
            // underlying transport has been closed. This condition usually
            // suggests, however, that the remote end has been closed.
            ExceptionRef e = new Exception(
               "Could not perform SSL handshake. Socket closed.",
               SOCKET_EXCEPTION_TYPE ".SslHandshakeError");
            Exception::set(e);
            rval = false;
            break;
         }
         case SSL_ERROR_WANT_READ:
         case SSL_ERROR_WANT_WRITE:
         {
            // transport data over underlying socket
            ret = _tcpTransport(mSocket, mSocketBio);
            if(ret <= 0)
            {
               ExceptionRef e = new Exception(
                  "Could not perform SSL handshake. Socket closed.",
                  SOCKET_EXCEPTION_TYPE ".SslHandshakeError");
               (ret < 0) ? Exception::push(e) : Exception::set(e);
               rval = false;
            }
            break;
         }
         default:
         {
            // an error occurred
            ExceptionRef e = new Exception(
               "Could not perform SSL handshake.",
               SOCKET_EXCEPTION_TYPE ".SslHandshakeError");
            e->getDetails()["error"] = SslContext::getSslErrorStrings();
            Exception::set(e);
            rval = false;
            break;
         }
      }
   }

   return rval;
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
         SOCKET_EXCEPTION_TYPE ".Closed");
      Exception::set(e);
      rval = false;
   }
   else
   {
      // do SSL_write() (implicit handshake performed as necessary)
      int ret = 0;
      while(rval && (ret = SSL_write(mSSL, b, length)) <= 0)
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
                  SOCKET_EXCEPTION_TYPE ".Closed");
               e->getDetails()["error"] = SslContext::getSslErrorStrings();
               Exception::set(e);
               rval = false;
               break;
            }
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
               // transport data over underlying socket
               ret = _tcpTransport(mSocket, mSocketBio);
               if(ret <= 0)
               {
                  // the connection was shutdown
                  ExceptionRef e = new Exception(
                     "Could not write to socket. Socket closed.",
                     SOCKET_EXCEPTION_TYPE ".Closed");
                  e->getDetails()["error"] = strerror(errno);
                  (ret < 0) ? Exception::push(e) : Exception::set(e);
                  rval = false;
               }
               break;
            default:
            {
               // an error occurred
               ExceptionRef e = new Exception(
                  "Could not write to socket.",
                  SOCKET_EXCEPTION_TYPE ".WriteError");
               e->getDetails()["error"] = SslContext::getSslErrorStrings();
               Exception::set(e);
               rval = false;
               break;
            }
         }
      }

      // flush all data to the socket
      rval = rval && (_tcpTransport(mSocket, mSocketBio) != -1);
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
         SOCKET_EXCEPTION_TYPE ".Closed");
      Exception::set(e);
      rval = -1;
   }
   else
   {
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
               // transport data over underlying socket
               ret = _tcpTransport(mSocket, mSocketBio);
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
                  Exception::push(e);
                  rval = -1;
               }
               break;
            case SSL_ERROR_WANT_WRITE:
               // transport data over underlying socket
               rval = _tcpTransport(mSocket, mSocketBio);
               break;
            default:
            {
               // an error occurred
               ExceptionRef e = new Exception(
                  "Could not read from socket.",
                  SOCKET_EXCEPTION_TYPE ".ReadError");
               e->getDetails()["error"] = SslContext::getSslErrorStrings();
               Exception::set(e);
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
