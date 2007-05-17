/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketAddress.h"
#include "SocketDefinitions.h"

using namespace std;
using namespace db::net;

SocketAddress::SocketAddress()
{
   mAddress = "0.0.0.0";
   mPort = 0;
}

SocketAddress::SocketAddress(const string& address, unsigned short port) 
{
   mAddress = address;
   mPort = port;
}

SocketAddress::~SocketAddress()
{
}

void SocketAddress::setAddress(const string& address)
{
   mAddress = address;
}

const string& SocketAddress::getAddress()
{
   return mAddress;
}

void SocketAddress::setPort(unsigned short port)
{
   mPort = port;
}

unsigned short SocketAddress::getPort()
{
   return mPort;
}
