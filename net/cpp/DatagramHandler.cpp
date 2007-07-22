/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DatagramHandler.h"

using namespace db::net;

DatagramHandler::DatagramHandler(InternetAddress* address, bool cleanup)
{
   mAddress = address;
   mCleanup = cleanup;
}

DatagramHandler::~DatagramHandler()
{
   if(mCleanup)
   {
      delete mAddress;
   }
}

void DatagramHandler::run()
{
   // FIXME:
}

InternetAddress* DatagramHandler::getAddress()
{
   return mAddress;
}
