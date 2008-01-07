/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/UdpSocket.h"
#include "db/net/SocketDefinitions.h"
#include "db/io/PeekInputStream.h"
#include "db/net/SocketInputStream.h"
#include "db/net/SocketOutputStream.h"

using namespace db::io;
using namespace db::net;
using namespace db::rt;

UdpSocket::UdpSocket()
{
}

UdpSocket::~UdpSocket()
{
}

bool UdpSocket::acquireFileDescriptor(const char* domain)
{
   bool rval = true;
   
   if(mFileDescriptor == -1)
   {
      // use PF_INET = "protocol family internet" (which just so happens to
      // have the same value as AF_INET but that's only because different
      // protocols were never used with the same address family
      if(strcmp(domain, "IPv6") == 0)
      {
         // use IPv6
         rval = create(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
      }
      else
      {
         // default to IPv4
         rval = create(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
      }
   }
   
   return rval;
}

Socket* UdpSocket::createConnectedSocket(unsigned int fd)
{
   // create a new UdpSocket
   UdpSocket* socket = new UdpSocket();
   socket->mFileDescriptor = fd;
   socket->mBound = true;
   socket->mConnected = true;
   
   // initialize input and output
   socket->initializeInput();
   socket->initializeOutput();
   
   return socket;
}

bool UdpSocket::joinGroup(SocketAddress* group, SocketAddress* localAddress)
{
   int error = 0;
   
   if(strcmp(group->getProtocol(), "IPv6") == 0)
   {
      // create IPv6 multicast request
      struct ipv6_mreq request;
      
      // set multicast address
      inet_pton(AF_INET6, group->getAddress(), &request.ipv6mr_multiaddr);
      
      // use any address for local interface
      request.ipv6mr_interface = 0;
      
      // join group
      error = setsockopt(
         mFileDescriptor, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
         (char*)&request, sizeof(request));
   }
   else
   {
      // create IPv4 multicast request
      struct ip_mreq request;
      
      // set multicast address
      inet_pton(AF_INET, group->getAddress(), &request.imr_multiaddr);
      
      // set local interface
      if(localAddress == NULL)
      {
         // use any address for local interface
         request.imr_interface.s_addr = INADDR_ANY;
      }
      else
      {
         inet_pton(AF_INET, localAddress->getAddress(), &request.imr_interface);
      }
      
      // join group
      error = setsockopt(
         mFileDescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,
         (char*)&request, sizeof(request));
   }
   
   if(error < 0)
   {
      Exception::setLast(new SocketException(
         "Could not join multicast group!", strerror(errno)));
   }
   
   return error == 0;
}

bool UdpSocket::leaveGroup(SocketAddress* group)
{
   int error = 0;
   
   if(strcmp(group->getProtocol(), "IPv6") == 0)
   {
      // create IPv6 multicast request
      struct ipv6_mreq request;
      
      // set multicast address
      inet_pton(AF_INET6, group->getAddress(), &request.ipv6mr_multiaddr);
      
      // use any address for local interface
      request.ipv6mr_interface = 0;
      
      // leave group
      error = setsockopt(
         mFileDescriptor, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP,
         (char*)&request, sizeof(request));
   }
   else
   {
      // create IPv4 multicast request
      struct ip_mreq request;
      
      // set multicast address
      inet_pton(AF_INET, group->getAddress(), &request.imr_multiaddr);
      
      // use any address for local interface
      request.imr_interface.s_addr = INADDR_ANY;
      
      // leave group
      error = setsockopt(
         mFileDescriptor, IPPROTO_IP, IP_DROP_MEMBERSHIP,
         (char*)&request, sizeof(request));
   }
   
   if(error < 0)
   {
      Exception::setLast(new SocketException(
         "Could not leave multicast group!", strerror(errno)));
   }
   
   return error == 0;
}

bool UdpSocket::sendDatagram(const char* b, int length, SocketAddress* address)
{
   Exception* exception = NULL;
   
   if(!isBound())
   {
      exception = new SocketException("Cannot write to unbound Socket!");
   }
   else
   {
      // populate address structure
      unsigned int size = 130;
      char addr[size];
      address->toSockAddr((sockaddr*)&addr, size);
      
      // send all data (send can fail to send all bytes in one go because the
      // socket send buffer was full)
      unsigned int offset = 0;
      while(length > 0 && exception == NULL)
      {
         // wait for socket to become writable
         if(select(false, getSendTimeout()))
         {
            int bytes = ::sendto(
               mFileDescriptor, b + offset, length, 0, (sockaddr*)&addr, size);
            if(bytes < 0)
            {
               exception = new SocketException(
                  "Could not write to Socket!", strerror(errno));
            }
            else if(bytes > 0)
            {
               offset += bytes;
               length -= bytes;
            }
         }
         else
         {
            exception = Exception::getLast();
         }
      }
   }
   
   return exception == NULL;
}

int UdpSocket::receiveDatagram(char* b, int length, SocketAddress* address)
{
   int rval = -1;
   
   if(!isBound())
   {
      Exception::setLast(new SocketException(
         "Cannot read from unbound Socket!"));
   }
   else if(select(true, getReceiveTimeout()))
   {
      // get address structure
      socklen_t size = 130;
      char addr[size];
      
      // receive some data
      rval = ::recvfrom(mFileDescriptor, b, length, 0, (sockaddr*)&addr, &size);
      if(rval < -1)
      {
         rval = -1;
         Exception::setLast(new SocketException(
            "Could not read from Socket!", strerror(errno)));
      }
      else if(rval != 0 && address != NULL)
      {
         // convert socket address
         address->fromSockAddr((sockaddr*)&addr, size);
      }
   }
   
   return rval;
}

bool UdpSocket::setMulticastHops(unsigned char hops)
{
   int error = 0;
   
   // set multicast hops flag
   error = setsockopt(
      mFileDescriptor, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
      (char*)&hops, sizeof(hops));
   
   if(error < 0)
   {
      Exception::setLast(new SocketException(
         "Could not set multicast hops!", strerror(errno)));
   }
   
   return error == 0;
}

bool UdpSocket::setMulticastTimeToLive(unsigned char ttl)
{
   int error = 0;
   
   // set multicast ttl flag
   error = setsockopt(
      mFileDescriptor, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));
   
   if(error < 0)
   {
      Exception::setLast(new SocketException(
         "Could not set multicast TTL!", strerror(errno)));
   }
   
   return error == 0;
}

bool UdpSocket::setBroadcastEnabled(bool enable)
{
   // set broadcast flag
   int broadcast = (enable) ? 1 : 0;
   int error = setsockopt(
      mFileDescriptor, SOL_SOCKET, SO_BROADCAST,
      (char *)&broadcast, sizeof(broadcast));
   if(error < 0)
   {
      Exception::setLast(new SocketException(
         "Could not set broadcast flag!", strerror(errno)));
   }
   
   return error == 0;
}
