/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/SocketDefinitions.h"
#include "db/net/SslContext.h"
#include "db/rt/DynamicObject.h"

#include <openssl/err.h>

using namespace db::io;
using namespace db::net;
using namespace db::rt;

SslContext::SslContext(const char* protocol, bool client)
{
   if(protocol == NULL || strcmp(protocol, "ALL") == 0)
   {
      // use all available protocols
      mContext = SSL_CTX_new(SSLv23_method());
   }
   else if(strcmp(protocol, "SSLv2") == 0)
   {
      // use only SSLv2
      mContext = SSL_CTX_new(SSLv2_method());
   }
   else if(strcmp(protocol, "SSLv3") == 0)
   {
      // use only SSLv3
      mContext = SSL_CTX_new(SSLv3_method());
   }
   else if(strcmp(protocol, "SSLv23") == 0)
   {
      // use SSLv2 or SSLv3
      mContext = SSL_CTX_new(SSLv23_method());
   }
   else if(strcmp(protocol, "TLS") == 0)
   {
      // use only TLS
      mContext = SSL_CTX_new(TLSv1_method());
   }
   
   // turn on all options (this enables a bunch of bug fixes for various
   // SSL implementations that may communicate with sockets created in
   // this context)
   SSL_CTX_set_options(mContext, SSL_OP_ALL);
   
   // cache server sessions so if a client proposes a session it can
   // be found in the cache and re-used
   SSL_CTX_set_session_cache_mode(mContext, SSL_SESS_CACHE_SERVER);
   
   // set SSL session context ID
   const char* id = "DBSSLCTXID";
   SSL_CTX_set_session_id_context(
      mContext, (const unsigned char*)id, strlen(id));
   
   // default to peer authentication only for the client (which means
   // only a client will check a server's cert, a server will not
   // request any client cert -- which is the common behavior)
   setPeerAuthentication(client);
   
   // use default ciphers
   // Note: even if non-authenticating ciphers (i.e. "aNULL") are
   // chosen here, you will get a "no shared cipher" error unless
   // openssl is specifically built allowing non-authenticating
   // ciphers -- EVEN IF -- the cipher name shows up in the list
   // of ciphers
   SSL_CTX_set_cipher_list(mContext, "DEFAULT");
}

SslContext::~SslContext()
{
   // free context
   if(mContext != NULL)
   {
      SSL_CTX_free(mContext);
   }
}

SSL* SslContext::createSSL(TcpSocket* socket, bool client)
{
   mLock.lock();
   SSL* ssl = SSL_new(mContext);
   mLock.unlock();
   
   // set connect state on SSL
   if(client)
   {
      SSL_set_connect_state(ssl);
   }
   else
   {
      SSL_set_accept_state(ssl);
   }
   
   return ssl;
}

bool SslContext::setCertificate(File& certFile)
{
   bool rval = true;
   
   // set certificate file
   if(SSL_CTX_use_certificate_file(
      mContext, certFile->getAbsolutePath(), SSL_FILETYPE_PEM) != 1)
   {
      // an error occurred
      ExceptionRef e = new Exception(
         "Could not set SSL certificate!",
         SSL_EXCEPTION_TYPE);
      e->getDetails()["error"] = SslContext::getSslErrorStrings();
      Exception::setLast(e, false);
      rval = false;
   }
   
   return rval;
}

bool SslContext::setPrivateKey(File& pkeyFile)
{
   bool rval = true;
   
   // set private key file
   if(SSL_CTX_use_PrivateKey_file(
      mContext, pkeyFile->getAbsolutePath(), SSL_FILETYPE_PEM) != 1)
   {
      // an error occurred
      ExceptionRef e = new Exception(
         "Could not set SSL private key!",
         SSL_EXCEPTION_TYPE);
      Exception::setLast(e, false);
      rval = false;
   }
   
   return rval;
}

void SslContext::setPeerAuthentication(bool on)
{
   SSL_CTX_set_verify(
      mContext, (on) ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);
}

bool SslContext::setVerifyCAs(File* caFile, File* caDir)
{
   bool rval = true;
   
   // load verify locations
   if(SSL_CTX_load_verify_locations(
      mContext,
      (caFile != NULL) ? (*caFile)->getAbsolutePath() : NULL,
      (caDir != NULL) ? (*caDir)->getAbsolutePath() : NULL) != 1)
   {
      // an error occurred
      ExceptionRef e = new Exception(
         "Could not set verify Certificate Authorities!",
         SSL_EXCEPTION_TYPE);
      Exception::setLast(e, false);
      rval = false;
   }
   
   return rval;
}

DynamicObject SslContext::getSslErrorStrings()
{
   DynamicObject rval;
   
   rval->setType(Array);
   long err;
   while((err = ERR_get_error()) != 0)
   {
      rval->append() = ERR_error_string(err, NULL);
   }
   
   return rval;
}
