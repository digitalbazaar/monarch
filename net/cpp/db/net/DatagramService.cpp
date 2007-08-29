/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/DatagramService.h"

using namespace db::modest;
using namespace db::net;

DatagramService::DatagramService(
   Server* server, InternetAddress* address, DatagramServicer* servicer) :
   PortService(server, address)
{
   mSocket = NULL;
}

DatagramService::~DatagramService()
{
   // ensure service is stopped
   DatagramService::stop();
}

Operation* DatagramService::initialize()
{
   Operation* rval = NULL;
   
   // create datagram socket
   mSocket = new DatagramSocket();
   
   // bind socket to the address
   if(mSocket->bind(getAddress()))
   {
      // create Operation for running service
      rval = new Operation(this, NULL, NULL);
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
