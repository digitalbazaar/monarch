/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_SslSession_H
#define db_net_SslSession_H

#include "db/rt/Collectable.h"

#include <openssl/ssl.h>

namespace db
{
namespace net
{

/**
 * The SslSessionImpl is a simple container that will store an
 * SSL_SESSION and free it when it is destructed.
 * 
 * @author Dave Longley
 */
class SslSessionImpl
{
public:
   SSL_SESSION* session;
   SslSessionImpl(SSL_SESSION* s = NULL)
   {
      session = s;
   }
   
   virtual ~SslSessionImpl()
   {
      SSL_SESSION_free(session);
   }
};

/**
 * The SslSession type is a reference counted container for an SSL_SESSION.
 */
typedef db::rt::Collectable<SslSessionImpl> SslSession;

} // end namespace net
} // end namespace db
#endif
