/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ConnectionGuard.h"
#include "Convert.h"

using namespace db::modest;
using namespace db::net;
using namespace db::util;

ConnectionGuard::ConnectionGuard(Server* s, ConnectionHandler* h)
{
   mServer = s;
   mHandler = h;
}

ConnectionGuard::~ConnectionGuard()
{
}

bool ConnectionGuard::canExecuteOperation(ImmutableState* s)
{
   bool rval = true;
   
   // FIXME: decide how to handle accepting vs. handling connection
   // protocol detection, etc (i.e. which operation is which, how to
   // implement guards)
   
   int max = 0;
   if(s->getInteger("com.db.net.server.connections.maxCount", max))
   {
      // ensure that the maximum number of connections won't been exceeded
      int count = mServer->getConnectionCount();
      if(count < max)
      {
         rval = true;
      }
   }
   
   if(rval && s->getInteger(
      "com.db.net.server.connections.ports." +
      Convert::integerToString(mHandler->getAddress()->getPort()) +
      ".maxCount", max))
   {
      // ensure that the maximum number of connections won't been exceeded
      int count = mHandler->getConnectionCount();
      if(count < max)
      {
         rval = true;
      }
   }
   
   return rval;
}

bool ConnectionGuard::mustCancelOperation(ImmutableState* s)
{
   // FIXME:
   return true;
}
