/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/SocketAddress.h"

#include "db/net/SocketDefinitions.h"
#include "db/rt/Exception.h"

#include <cstdlib>
#include <cstring>

using namespace std;
using namespace db::net;
using namespace db::rt;

SocketAddress::SocketAddress(
   CommunicationDomain domain, const char* address, unsigned short port) :
   mCommDomain(domain),
   mAddress(strdup(address)),
   mPort(port)
{
}

SocketAddress::~SocketAddress()
{
   free(mAddress);
}

void SocketAddress::setCommunicationDomain(CommunicationDomain domain)
{
   mCommDomain = domain;
}

SocketAddress::CommunicationDomain SocketAddress::getCommunicationDomain()
{
   return mCommDomain;
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

string SocketAddress::toString(bool simple)
{
   string rval;

   if(simple)
   {
      char temp[6 + strlen(getAddress())];
      sprintf(temp, "%s:%u", getAddress(), getPort());
      rval = temp;
   }
   else
   {
      char temp[50 + strlen(getAddress())];
      sprintf(temp, "SocketAddress [%s:%u]", getAddress(), getPort());
      rval = temp;
   }

   return rval;
}

bool SocketAddress::fromString(const char* str)
{
   bool rval = true;

   // look for colon separator
   const char* colon = strchr(str, ':');
   if(colon != NULL && str[0] != ':' && strlen(colon) > 0)
   {
      // get port
      unsigned int port = strtoul(colon + 1, NULL, 10);
      mPort = (port & 0xFFFF);

      // get address
      int len = (colon - str);
      char address[len + 1];
      address[len] = 0;
      memcpy(address, str, len);
      setAddress(address);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not parse SocketAddress from string.",
         "db.net.SocketAddress.ParseError");
      e->getDetails()["string"] = str;
      Exception::set(e);
      rval = false;
   }

   return rval;
}

const char* SocketAddress::communicationDomainToString(
   CommunicationDomain domain)
{
   const char* rval;

   switch(domain)
   {
      case IPv4:
         rval = "IPv4";
         break;
      case IPv6:
         rval = "IPv6";
         break;
      default:
         // should never happen
         rval = "invalid";
         break;
   }

   return rval;
}
