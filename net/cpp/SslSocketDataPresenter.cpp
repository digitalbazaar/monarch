/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SslSocketDataPresenter.h"
#include "SslSocket.h"

using namespace db::net;

SslSocketDataPresenter::SslSocketDataPresenter(SslContext* context)
{
   mContext = context;
}

SslSocketDataPresenter::~SslSocketDataPresenter()
{
}

Socket* SslSocketDataPresenter::createPresentationWrapper(
   Socket* s, bool& secure)
{
   SslSocket* rval = NULL;
   
   // FIXME: detect SSL packet
   // if(detectSsl(s))
   if(true)
   {
      // create an SSL socket, (false = use server mode, true = cleanup)
      rval = new SslSocket(mContext, (TcpSocket*)s, false, true);
   }
   
   return rval;
}
