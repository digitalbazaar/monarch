/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/SslContext.h"

#include "monarch/logging/Logging.h"
#include "monarch/net/SocketDefinitions.h"
#include "monarch/rt/DynamicObject.h"

#include <openssl/err.h>

using namespace monarch::crypto;
using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;

SslContext::SslContext(const char* protocol, bool client) :
   mVirtualHost(NULL),
   mPrivateKey(NULL),
   mCertificate(NULL)
{
   if(protocol == NULL || strcmp(protocol, "ALL") == 0)
   {
      // use all available protocols
      mContext = SSL_CTX_new(SSLv23_method());

      // disable SSLv2 by default because it has poor security
      SSL_CTX_set_options(mContext, SSL_OP_NO_SSLv2);
   }
   else if(strcmp(protocol, "TLS") == 0)
   {
      // use only TLS
      mContext = SSL_CTX_new(TLSv1_method());
   }
   else if(strcmp(protocol, "SSLv3") == 0)
   {
      // use only SSLv3
      mContext = SSL_CTX_new(SSLv3_method());
   }
   else if(strcmp(protocol, "ALL+2") == 0)
   {
      // use all available protocols (allow SSLv2)
      mContext = SSL_CTX_new(SSLv23_method());
   }
   else if(strcmp(protocol, "SSLv23") == 0)
   {
      // use SSLv2 or SSLv3
      mContext = SSL_CTX_new(SSLv23_method());
   }
   else if(strcmp(protocol, "SSLv2") == 0)
   {
      // use only SSLv2
      mContext = SSL_CTX_new(SSLv2_method());
   }

   // turn on all options (this enables a bunch of bug fixes for various
   // SSL implementations that may communicate with sockets created in
   // this context)
   SSL_CTX_set_options(mContext, SSL_OP_ALL);

   // cache server sessions so if a client proposes a session it can
   // be found in the cache and re-used
   SSL_CTX_set_session_cache_mode(mContext, SSL_SESS_CACHE_SERVER);

   // FIXME: this should be done with a per server/per app ID
   // set SSL session context ID
   const char* id = "MOSSLCTXID";
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
   // clean up all virtual hosts
   while(!mVirtualHosts.empty())
   {
      VirtualHostMap::iterator i = mVirtualHosts.begin();
      SslContextRef ctx = i->second;
      mVirtualHosts.erase(i);
      ctx.setNull();
   }

   // free context
   if(mContext != NULL)
   {
      SSL_CTX_free(mContext);
   }

   // free virtual hostname
   if(mVirtualHost != NULL)
   {
      free(mVirtualHost);
   }
}

SSL* SslContext::createSSL(TcpSocket* socket, bool client)
{
   mContextLock.lock();
   SSL* ssl = SSL_new(mContext);
   mContextLock.unlock();

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

void SslContext::setVirtualHost(const char* vHost)
{
   if(mVirtualHost != NULL)
   {
      free(mVirtualHost);
   }
   mVirtualHost = strdup(vHost);
}

const char* SslContext::getVirtualHost()
{
   return mVirtualHost;
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
         "Could not set SSL certificate.",
         SSL_EXCEPTION_TYPE);
      e->getDetails()["filename"] = certFile->getAbsolutePath();
      e->getDetails()["error"] = SslContext::getSslErrorStrings();
      Exception::set(e);
      rval = false;
   }
   else
   {
      // clear any old certificate
      mCertificate.setNull();
   }

   return rval;
}

bool SslContext::setCertificate(X509CertificateRef& cert)
{
   bool rval = true;

   // set certificate
   if(SSL_CTX_use_certificate(mContext, cert->getX509()) != 1)
   {
      // an error occurred
      ExceptionRef e = new Exception(
         "Could not set SSL certificate.",
         SSL_EXCEPTION_TYPE);
      e->getDetails()["error"] = SslContext::getSslErrorStrings();
      Exception::set(e);
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
         "Could not set SSL private key.",
         SSL_EXCEPTION_TYPE);
      e->getDetails()["filename"] = pkeyFile->getAbsolutePath();
      e->getDetails()["error"] = SslContext::getSslErrorStrings();
      Exception::set(e);
      rval = false;
   }
   else
   {
      // clear any old private key
      mPrivateKey.setNull();
   }

   return rval;
}

bool SslContext::setPrivateKey(PrivateKeyRef& pkey)
{
   bool rval = true;

   // set private key
   if(SSL_CTX_use_PrivateKey(mContext, pkey->getPKEY()) != 1)
   {
      // an error occurred
      ExceptionRef e = new Exception(
         "Could not set SSL private key.",
         SSL_EXCEPTION_TYPE);
      e->getDetails()["error"] = SslContext::getSslErrorStrings();
      Exception::set(e);
      rval = false;
   }

   return rval;
}

int _sniCallback(SSL* s, int* al, void* arg)
{
   SslContext* sc = static_cast<SslContext*>(arg);
   return sc->handleSni(s);
}

bool SslContext::addVirtualHost(SslContextRef& ctx)
{
   bool rval = true;

   if(ctx->getVirtualHost() == NULL)
   {
      ExceptionRef e = new Exception(
         "Could not add virtual host. No virtual hostname set on the "
         "given context.",
         SSL_EXCEPTION_TYPE ".NullVirtualHost");
      Exception::set(e);
      rval = false;
   }
   else
   {
      const char* name = ctx->getVirtualHost();
      mVirtualHostLock.lockExclusive();
      VirtualHostMap::iterator i = mVirtualHosts.find(name);
      if(i != mVirtualHosts.end())
      {
         mVirtualHostLock.unlockExclusive();
         ExceptionRef e = new Exception(
            "Could not add virtual host. Entry already exists.",
            SSL_EXCEPTION_TYPE ".DuplicateVirtualHost");
         e->getDetails()["name"] = name;
         Exception::set(e);
         rval = false;
      }
      else
      {
         if(mVirtualHosts.size() == 0)
         {
            // set SNI callback
            rval =
               SSL_CTX_set_tlsext_servername_callback(mContext, _sniCallback) &&
               SSL_CTX_set_tlsext_servername_arg(mContext, this);
         }

         // add the virtual host entry
         mVirtualHosts[name] = ctx;
         mVirtualHostLock.unlockExclusive();
      }
   }

   return rval;
}

bool SslContext::removeVirtualHost(const char* name, SslContextRef* ctx)
{
   bool rval = true;

   mVirtualHostLock.lockExclusive();
   VirtualHostMap::iterator i = mVirtualHosts.find(name);
   if(i == mVirtualHosts.end())
   {
      mVirtualHostLock.unlockExclusive();
      ExceptionRef e = new Exception(
         "Could not remove virtual host. Entry not found.",
         SSL_EXCEPTION_TYPE ".VirtualHostNotFound");
      e->getDetails()["name"] = name;
      Exception::set(e);
      rval = false;
   }
   else
   {
      // remove entry from map
      SslContextRef oldCtx = i->second;
      mVirtualHosts.erase(i);
      mVirtualHostLock.unlockExclusive();

      // save context
      if(ctx != NULL)
      {
         *ctx = oldCtx;
      }

      // clean up entry
      oldCtx.setNull();
   }

   return rval;
}

int SslContext::handleSni(SSL* s)
{
   int rval = SSL_TLSEXT_ERR_OK;

   // get the server name from TLS SNI extension
   const char* name = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
   if(name)
   {
      // try to find a matching virtual host
      mVirtualHostLock.lockShared();
      VirtualHostMap::iterator i = mVirtualHosts.find(name);
      if(i != mVirtualHosts.end())
      {
         MO_CAT_DEBUG(MO_NET_CAT,
            "Using TLS SNI virtual host '%s'", name);

         // switch contexts and copy options
         SSL_CTX* ctx = i->second->mContext;
         SSL_set_SSL_CTX(s, ctx);
         SSL_set_options(s, SSL_CTX_get_options(ctx));
      }
      // see if default virtual host does not match
      else if(mVirtualHost == NULL || strcmp(mVirtualHost, name) != 0)
      {
         MO_CAT_DEBUG(MO_NET_CAT,
            "TLS SNI virtual host '%s' not found, using default host", name);

         // issue a warning, but keep the same context
         rval = SSL_TLSEXT_ERR_ALERT_WARNING;
      }
      mVirtualHostLock.unlockShared();
   }
   else
   {
      // no name sent, do not acknowledge, just use default host
      rval = SSL_TLSEXT_ERR_NOACK;
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
         "Could not set verify Certificate Authorities.",
         SSL_EXCEPTION_TYPE);
      if(caFile != NULL)
      {
         e->getDetails()["filename"] = (*caFile)->getAbsolutePath();
      }
      else if(caDir != NULL)
      {
         e->getDetails()["directory"] = (*caDir)->getAbsolutePath();
      }
      Exception::set(e);
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
