/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ConnectionHandler.h"

using namespace db::net;

ConnectionHandler::ConnectionHandler(InternetAddress* address, bool cleanup)
{
   mAddress = address;
   mCleanup = cleanup;
   mConnectionCount = 0;
}

ConnectionHandler::~ConnectionHandler()
{
   if(mCleanup)
   {
      delete mAddress;
   }
}

unsigned long ConnectionHandler::getConnectionCount()
{
   return mConnectionCount;
}

InternetAddress* ConnectionHandler::getAddress()
{
   return mAddress;
}
