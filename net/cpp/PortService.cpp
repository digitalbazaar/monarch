/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "PortService.h"
#include "Server.h"
#include "Convert.h"

using namespace std;
using namespace db::modest;
using namespace db::net;
using namespace db::util;

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

string& PortService::toString(string& str)
{
   string port = Convert::integerToString(mAddress->getPort());
   str = "PortService [" + mAddress->getHost() + ":" + port + "]";
   return str;
}
