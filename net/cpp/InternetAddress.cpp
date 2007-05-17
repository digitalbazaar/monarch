/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "InternetAddress.h"
#include "SocketDefinitions.h"

using namespace std;
using namespace db::net;

InternetAddress::InternetAddress()
{
   // set protocol
   setProtocol("IPv4");
}

InternetAddress::InternetAddress(const string& host, unsigned short port)
throw(UnknownHostException)
{
   // set protocol
   setProtocol("IPv4");
   
   // resolve host
   setHost(host);
   
   // set port
   setPort(port);
}

InternetAddress::~InternetAddress()
{
}

bool InternetAddress::toSockAddr(sockaddr* addr, unsigned int& size)
{
   bool rval = false;
   
   // use sockaddr_in (IPv4)
   if(size >= sizeof(sockaddr_in))
   {
      struct sockaddr_in* sa = (sockaddr_in*)addr;
      size = sizeof(sockaddr_in);
      memset(sa, '\0', size);
      
      // the address family is internet (AF_INET = address family internet)
      sa->sin_family = AF_INET;
      
      // htons = "Host To Network Short" which means order the short in
      // network byte order (big-endian)
      sa->sin_port = htons(getPort());
      
      // converts an address to network byte order
      rval = (inet_pton(AF_INET, getAddress().c_str(), &sa->sin_addr) == 1);
   }
   
   return rval;
}

bool InternetAddress::fromSockAddr(const sockaddr* addr, unsigned int size)
{
   bool rval = false;
   
   // use sockaddr_in (IPv4)
   if(size >= sizeof(sockaddr_in))
   {
      struct sockaddr_in* sa = (sockaddr_in*)addr;
      
      // get address
      char dst[32];
      memset(&dst, '\0', 32);
      if(inet_ntop(AF_INET, &sa->sin_addr, dst, 32) != NULL)
      {
         // set address
         setAddress(dst);
         
         // converting from network byte order to little-endian
         setPort(ntohs(sa->sin_port));
         
         // conversion successful
         rval = true;
      }
   }
   
   return rval;
}

void InternetAddress::setAddress(const string& address)
{
   // set the address
   mAddress = address;
   
   // clear the host
   mHost = "";
}

void InternetAddress::setHost(const std::string& host)
throw(UnknownHostException)
{
   // create hints address structure
   struct addrinfo hints;
   memset(&hints, '\0', sizeof(hints));
   hints.ai_family = AF_INET;
   
   // create pointer for storing allocated resolved address
   struct addrinfo* res = NULL;
   
   // get address information
   if(getaddrinfo(host.c_str(), NULL, &hints, &res) != 0)
   {
      throw UnknownHostException("Unknown host '" + host + "'!");
   }
   
   // copy the first result
   struct sockaddr_in addr;
   memcpy(&addr, res->ai_addr, res->ai_addrlen);
   
   // get the address
   char dst[32];
   memset(&dst, '\0', 32);
   inet_ntop(AF_INET, &addr.sin_addr, dst, 32);
   mAddress = dst;
   
   // free result
   freeaddrinfo(res);
}

const string& InternetAddress::getHost()
{
   if(mHost == "" && getAddress() != "")
   {
      // get a IPv4 address structure
      struct sockaddr_in sa;
      unsigned int size = sizeof(sockaddr_in);
      toSockAddr((sockaddr*)&sa, size);
      
      // NULL specifies that we don't care about getting a "service" name
      // given in sockaddr_in will be returned
      char dst[100];
      memset(&dst, '\0', 100);
      getnameinfo((sockaddr*)&sa, size, dst, 100, NULL, 0, 0);
      
      // set host name
      mHost = dst;
   }
   
   // return host
   return mHost;
}

bool InternetAddress::isMulticast()
{
   bool rval = false;
   
   // get a IPv4 address structure
   struct sockaddr_in sa;
   unsigned int size = sizeof(sockaddr_in);
   toSockAddr((sockaddr*)&sa, size);
   
   if(IN_MULTICAST(ntohl(sa.sin_addr.s_addr)) != 0)
   {
      rval = true;
   }
   
   return rval;
}
