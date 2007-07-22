/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DatagramService.h"

using namespace db::net;

DatagramService::DatagramService(
   InternetAddress* address, DatagramServicer* servicer)
{
   mAddress = address;
}

DatagramService::~DatagramService()
{
}

void DatagramService::run()
{
   // create datagram socket
   DatagramSocket s;
   
   // bind socket to the address
   if(s.bind(mAddress))
   {
      // service datagrams
      mServicer->serviceDatagrams(&s);
   }
   
   // close socket
   s.close();
}

InternetAddress* DatagramService::getAddress()
{
   return mAddress;
}
