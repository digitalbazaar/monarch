/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SslContext.h"

using namespace db::net;

SslContext::SslContext(const char* protocol)
{
   // FIXME: handle protocol "SSLv2/SSLv3/TLS"
   
   // create SSL context object
   //mContext = SSL_CTX_new(SSLv2_method());
   mContext = SSL_CTX_new(SSLv23_method());
   //mContext = SSL_CTX_new(SSLv3_method());
   //mContext = SSL_CTX_new(TLSv1_method());
   
   // turn on all options (this enables a bunch of bug fixes for various
   // SSL implementations that may communicate with sockets created in
   // this context)
   SSL_CTX_set_options(mContext, SSL_OP_ALL);
   
   // FIXME:
   // implement reusing SSL sessions to speed up connection initialization
   // either here or below
   //SSL_CTX_set_session_id_context(
   //   context, const unsigned char* serverIdContext,
   //   sizeof(serverIdContext));
   
   // FIXME: remove this after testing
   //SSL_CTX_set_verify(mContext, SSL_VERIFY_NONE, NULL);
   //SSL_CTX_set_cipher_list(mContext, "DEFAULT");
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
   SSL* ssl = SSL_new(mContext);
   
   // FIXME:
   // implement reusing SSL sessions to speed up connection initialization
   // either here or above
   //SSL_set_session_id_context(
   //   ssl, const unsigned char* serverIdContext,
   //   sizeof(serverIdContext));
   
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
