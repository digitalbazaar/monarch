/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/DatagramService.h"

#include "db/net/Server.h"

using namespace db::modest;
using namespace db::net;

DatagramService::DatagramService(
   Server* server, InternetAddress* address, DatagramServicer* servicer,
   const char* name) :
   PortService(server, address, name)
{
   mServicer = servicer;
   mSocket = NULL;
}

DatagramService::~DatagramService()
{
   // ensure service is stopped
   DatagramService::stop();
}

Operation DatagramService::initialize()
{
   Operation rval(NULL);

   // create datagram socket
   mSocket = new DatagramSocket();

   // bind socket to the address
   if(mSocket->bind(getAddress()) && mServicer->initialize(mSocket))
   {
      // create Operation for running service
      rval = *this;
   }

   return rval;
}

void DatagramService::cleanup()
{
   if(mSocket != NULL)
   {
      // clean up socket
      delete mSocket;
      mSocket = NULL;
   }
}

void DatagramService::run()
{
   // service datagrams
   mServicer->serviceDatagrams(mSocket);

   // close socket
   mSocket->close();
}
