/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "InternetAddress.h"
#include "SocketDefinitions.h"

using namespace std;
using namespace db::net;

InternetAddress::InternetAddress()
{
}

InternetAddress::InternetAddress(const string& address, unsigned short port)
{
   setAddress(address);
   mPort = port;
}

InternetAddress::~InternetAddress()
{
}

void InternetAddress::setAddress(const string& address) throw(SocketException)
{
   // store the address as the host
   mHost = address;
   
   // try to resolve the address
   struct hostent* host = gethostbyname(address.c_str());
   if(host == NULL)
   {
      throw SocketException("Unknown host: " + address + "!");
   }
   
   mHost = host->h_name;
   mAddress = host->h_addr;
}

const string& InternetAddress::getHost()
{
   return mHost;
}
