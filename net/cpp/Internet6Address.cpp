/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Internet6Address.h"
#include "SocketDefinitions.h"

using namespace std;
using namespace db::net;

Internet6Address::Internet6Address()
{
   // set protocol
   setProtocol("IPv6");
   
   // set default address and port
   mAddress = "::0";
   mPort = 0;
}

Internet6Address::Internet6Address(const string& host, unsigned short port)
throw(UnknownHostException)
{
   // set protocol
   setProtocol("IPv6");
   
   // resolve host
   setHost(host);
   
   // set port
   setPort(port);
}

Internet6Address::~Internet6Address()
{
}

void Internet6Address::toSockAddr(sockaddr* addr)
{
   // use sockaddr_in6 (IPv6)
   struct sockaddr_in6* sa = (sockaddr_in6*)addr;
   
   // clear the socket address
   memset(sa, '\0', sizeof(sa));
   
   // the address family is internet 6 (AF_INET = address family internet)
   sa->sin6_family = AF_INET6;
   
   // htons = "Host To Network Short" which means order the short in
   // network byte order (big-endian)
   sa->sin6_port = htons(getPort());
   
   // converts an address to network byte order
   inet_pton(AF_INET6, getAddress().c_str(), &sa->sin6_addr);
}

void Internet6Address::fromSockAddr(const sockaddr* addr)
{
   // use sockaddr_in6 (IPv6)
   struct sockaddr_in6* sa = (sockaddr_in6*)addr;
   
   // get address
   char dst[32];
   memset(&dst, '\0', 32);
   inet_ntop(AF_INET6, &sa->sin6_addr, dst, 32);
   setAddress(dst);
   
   // FIXME: handle converting from network byte order to little-endian
   setPort(sa->sin6_port);
}

void Internet6Address::setHost(const std::string& host)
throw(UnknownHostException)
{
   // create hints address structure
   struct addrinfo hints;
   memset(&hints, '\0', sizeof(hints));
   hints.ai_family = AF_INET6;
   
   // create pointer for storing allocated resolved address
   struct addrinfo* res = NULL;
   
   // get address information
   if(getaddrinfo(host.c_str(), NULL, &hints, &res) != 0)
   {
      throw UnknownHostException("Unknown host '" + host + "'!");
   }
   
   // copy the first result
   struct sockaddr_in6 addr;
   memcpy(&addr, res->ai_addr, res->ai_addrlen);
   
   // get the address
   char dst[32];
   memset(&dst, '\0', 32);
   inet_ntop(AF_INET6, &addr.sin6_addr, dst, 32);
   mAddress = dst;
   
   // free result
   freeaddrinfo(res);
}

const string& Internet6Address::getHost()
{
   if(mHost == "" && getAddress() != "")
   {
      // get a IPv6 address structure
      struct sockaddr_in6 sa;
      toSockAddr((sockaddr*)&sa);
      
      // NULL specifies that we don't care about getting a "service" name
      // given in sockaddr_in will be returned
      char dst[100];
      memset(&dst, '\0', 100);
      getnameinfo((sockaddr*)&sa, sizeof(sa), dst, 100, NULL, 0, 0);
      
      // set host name
      mHost = dst;
   }
   
   // return host
   return mHost;
}

bool Internet6Address::isMulticast()
{
   bool rval = false;
   
   struct sockaddr_in6 addr;
   toSockAddr((sockaddr*)&addr);
   
   if(IN6_IS_ADDR_MULTICAST(&addr.sin6_addr) != 0)
   {
      rval = true;
   }
   
   return rval;
}
