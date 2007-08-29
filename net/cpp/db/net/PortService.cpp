/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/PortService.h"
#include "db/net/Server.h"

using namespace db::modest;
using namespace db::net;

PortService::PortService(Server* server, InternetAddress* address)
{
   mServer = server;
   mAddress = address;
   mOperation = NULL;
}

PortService::~PortService()
{
}

bool PortService::start()
{
   if(mOperation != NULL)
   {
      // stop service
      stop();
   }
   
   // initialize service
   mOperation = initialize();
   if(mOperation != NULL)
   {
      // run service
      mServer->getKernel()->getEngine()->queue(mOperation);
   }
   else
   {
      // clean up service
      cleanup();
   }
   
   return mOperation != NULL;
}

void PortService::interrupt()
{
   if(mOperation != NULL)
   {
      // interrupt service
      mOperation->interrupt();
   }
}

void PortService::stop()
{
   if(mOperation != NULL)
   {
      // stop service
      mOperation->interrupt();
      mOperation->waitFor(false);
      delete mOperation;
      mOperation = NULL;
   }
   
   // clean up service
   cleanup();
}

InternetAddress* PortService::getAddress()
{
   return mAddress;
}
