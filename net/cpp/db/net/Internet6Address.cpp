/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/Internet6Address.h"

#include "db/net/WindowsSupport.h"
#include "db/rt/DynamicObject.h"
#include "db/rt/Exception.h"

#include <cstdlib>
#include <cstring>

using namespace db::net;
using namespace db::rt;

Internet6Address::Internet6Address(const char* host, unsigned short port) :
   InternetAddress("", port)
{
   // set domain
   Internet6Address::setCommunicationDomain(SocketAddress::IPv6);
   
   if(strcmp(host, "") != 0)
   {
      // resolve host
      Internet6Address::setHost(host);
   }
}

Internet6Address::~Internet6Address()
{
}

bool Internet6Address::toSockAddr(sockaddr* addr, unsigned int& size)
{
   bool rval = false;
   
   // use sockaddr_in6 (IPv6)
   if(size >= sizeof(sockaddr_in6))
   {
      struct sockaddr_in6* sa = (sockaddr_in6*)addr;
      size = sizeof(sockaddr_in6);
      memset(sa, '\0', size);
      
      // the address family is internet 6 (AF_INET = address family internet)
      sa->sin6_family = AF_INET6;
      
      // htons = "Host To Network Short" which means order the short in
      // network byte order (big-endian)
      sa->sin6_port = htons(getPort());
      
      // converts an address to network byte order
      rval = (inet_pton(AF_INET6, getAddress(), &sa->sin6_addr) == 1);
   }
   
   return rval;
}

bool Internet6Address::fromSockAddr(const sockaddr* addr, unsigned int size)
{
   bool rval = false;
   
   // use sockaddr_in6 (IPv6)
   if(size >= sizeof(sockaddr_in6))
   {
      struct sockaddr_in6* sa = (sockaddr_in6*)addr;
      
      // get address
      char dst[INET6_ADDRSTRLEN];
      memset(&dst, '\0', INET6_ADDRSTRLEN);
      if(inet_ntop(AF_INET6, &sa->sin6_addr, dst, INET6_ADDRSTRLEN) != NULL)
      {
         // set address
         setAddress(dst);
         
         // converting from network byte order to little-endian
         setPort(ntohs(sa->sin6_port));
         
         // conversion successful
         rval = true;
      }
   }
   
   return rval;
}

bool Internet6Address::setHost(const char* host)
{
   bool rval = false;
   
   // create hints address structure
   struct addrinfo hints;
   memset(&hints, '\0', sizeof(hints));
   hints.ai_family = AF_INET6;
   
   // create pointer for storing allocated resolved address
   struct addrinfo* res = NULL;
   
   // get address information
   if(getaddrinfo(host, NULL, &hints, &res) != 0)
   {
      ExceptionRef e = new Exception("Unknown host.", "db.net.UnknownHost");
      e->getDetails()["host"] = host;
      Exception::set(e);
   }
   else
   {
      // copy the first result
      struct sockaddr_in6 addr;
      memcpy(&addr, res->ai_addr, res->ai_addrlen);
      
      // get the address
      char dst[INET6_ADDRSTRLEN];
      memset(&dst, '\0', INET6_ADDRSTRLEN);
      inet_ntop(AF_INET6, &addr.sin6_addr, dst, INET6_ADDRSTRLEN);
      free(mAddress);
      mAddress = strdup(dst);
      rval = true;
      
      // save the host
      free(mHost);
      mHost = strdup(host);
   }
   
   if(res != NULL)
   {
      // free res if it got allocated
      freeaddrinfo(res);
   }
   
   return rval;
}

const char* Internet6Address::getHost()
{
   if(strcmp(mHost, "") == 0 && strcmp(getAddress(), "") != 0)
   {
      // get a IPv6 address structure
      struct sockaddr_in6 sa;
      unsigned int size = sizeof(sockaddr_in6);
      toSockAddr((sockaddr*)&sa, size);
      
      // NULL specifies that we don't care about getting a "service" name
      // given in sockaddr_in6 will be returned
      char dst[100];
      memset(&dst, '\0', 100);
      if(getnameinfo((sockaddr*)&sa, size, dst, 100, NULL, 0, 0) == 0)
      {
         // set host name
         free(mHost);
         mHost = strdup(dst);
      }
      else
      {
         // use address
         free(mHost);
         mHost = strdup(getAddress());
      }
   }
   
   // return host
   return mHost;
}

bool Internet6Address::isMulticast()
{
   bool rval = false;
   
   // get a IPv6 address structure
   struct sockaddr_in6 sa;
   unsigned int size = sizeof(sockaddr_in6);
   toSockAddr((sockaddr*)&sa, size);
   
   if(IN6_IS_ADDR_MULTICAST(&sa.sin6_addr) != 0)
   {
      rval = true;
   }
   
   return rval;
}
