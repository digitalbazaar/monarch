/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/SocketAddress.h"
#include "db/net/SocketDefinitions.h"

using namespace std;
using namespace db::net;

SocketAddress::SocketAddress(
   const char* protocol, const char* address, unsigned short port) 
{
   mProtocol = strdup(protocol);
   mAddress = strdup(address);
   mPort = port;
}

SocketAddress::~SocketAddress()
{
   free(mProtocol);
   free(mAddress);
}

void SocketAddress::setProtocol(const char* protocol)
{
   free(mProtocol);
   mProtocol = strdup(protocol);
}

const char* SocketAddress::getProtocol()
{
   return mProtocol;
}

void SocketAddress::setAddress(const char* address)
{
   free(mAddress);
   mAddress = strdup(address);
}

const char* SocketAddress::getAddress()
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

string SocketAddress::toString()
{
   char temp[50 + strlen(getAddress())];
   sprintf(temp, "SocketAddress [%s:%u]", getAddress(), getPort());
   return temp;
}
