/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/PortService.h"

#include "db/net/Server.h"

using namespace db::modest;
using namespace db::net;

PortService::PortService(Server* server, InternetAddress* address) :
   mOperation(NULL)
{
   mServer = server;
   mAddress = address;
}

PortService::~PortService()
{
}

bool PortService::start()
{
   if(!mOperation.isNull())
   {
      // stop service
      stop();
   }
   
   // initialize service
   mOperation = initialize();
   if(!mOperation.isNull())
   {
      // run service
      mServer->getOperationRunner()->runOperation(mOperation);
   }
   else
   {
      // clean up service
      cleanup();
   }
   
   return !mOperation.isNull();
}

void PortService::interrupt()
{
   if(!mOperation.isNull())
   {
      // interrupt service
      mOperation->interrupt();
   }
}

void PortService::stop()
{
   if(!mOperation.isNull())
   {
      // stop service
      mOperation->interrupt();
      mOperation->waitFor(false);
      mOperation.setNull();
   }
   
   // clean up service
   cleanup();
}

inline InternetAddress* PortService::getAddress()
{
   return mAddress;
}
