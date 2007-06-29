/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "UdpSocket.h"
#include "SocketDefinitions.h"
#include "PeekInputStream.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

using namespace std;
using namespace db::io;
using namespace db::net;
using namespace db::rt;

// FIXME: this class and others need to have another look taken at
// their exception code -- this class will fail to send/recv any
// datagrams when *any* exception is set, when this code should instead
// probably mirror other code that only checks for the exceptions it
// cares about

UdpSocket::UdpSocket()
{
}

UdpSocket::~UdpSocket()
{
}

bool UdpSocket::acquireFileDescriptor(const string& domain)
{
   bool rval = true;
   
   if(mFileDescriptor == -1)
   {
      // use PF_INET = "protocol family internet" (which just so happens to
      // have the same value as AF_INET but that's only because different
      // protocols were never used with the same address family
      if(domain == "IPv6")
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
   
   if(group->getProtocol() == "IPv6")
   {
      // create IPv6 multicast request
      struct ipv6_mreq request;
      
      // set multicast address
      inet_pton(
         AF_INET6, group->getAddress().c_str(), &request.ipv6mr_multiaddr);
      
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
      inet_pton(AF_INET, group->getAddress().c_str(), &request.imr_multiaddr);
      
      // set local interface
      if(localAddress == NULL)
      {
         // use any address for local interface
         request.imr_interface.s_addr = INADDR_ANY;
      }
      else
      {
         inet_pton(AF_INET, localAddress->getAddress().c_str(),
            &request.imr_interface);
      }
      
      // join group
      error = setsockopt(
         mFileDescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,
         (char*)&request, sizeof(request));
   }
   
   if(error < 0)
   {
      Thread::setException(new SocketException(
         "Could not join multicast group!", strerror(errno)));
   }
   
   return error == 0;
}

bool UdpSocket::leaveGroup(SocketAddress* group)
{
   int error = 0;
   
   if(group->getProtocol() == "IPv6")
   {
      // create IPv6 multicast request
      struct ipv6_mreq request;
      
      // set multicast address
      inet_pton(
         AF_INET6, group->getAddress().c_str(), &request.ipv6mr_multiaddr);
      
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
      inet_pton(AF_INET, group->getAddress().c_str(), &request.imr_multiaddr);
      
      // use any address for local interface
      request.imr_interface.s_addr = INADDR_ANY;
      
      // leave group
      error = setsockopt(
         mFileDescriptor, IPPROTO_IP, IP_DROP_MEMBERSHIP,
         (char*)&request, sizeof(request));
   }
   
   if(error < 0)
   {
      Thread::setException(new SocketException(
         "Could not leave multicast group!", strerror(errno)));
   }
   
   return error == 0;
}

bool UdpSocket::sendDatagram(
   const char* b, unsigned int length, SocketAddress* address)
{
   if(!isBound())
   {
      Thread::setException(new SocketException(
         "Cannot write to unbound Socket!"));
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
      while(length > 0 && !Thread::hasException())
      {
         // wait for socket to become writable
         if(select(false, getSendTimeout()))
         {
            int bytes = ::sendto(
               mFileDescriptor, b + offset, length, 0, (sockaddr*)&addr, size);
            if(bytes < 0)
            {
               Thread::setException(new SocketException(
                  "Could not write to Socket!", strerror(errno)));
            }
            else if(bytes > 0)
            {
               offset += bytes;
               length -= bytes;
            }
         }
      }
   }
   
   return !Thread::hasException();
}

int UdpSocket::receiveDatagram(
   char* b, unsigned int length, SocketAddress* address)
{
   int rval = -1;
   
   if(!isBound())
   {
      Thread::setException(new SocketException(
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
         Thread::setException(new SocketException(
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
      Thread::setException(new SocketException(
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
      Thread::setException(new SocketException(
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
      Thread::setException(new SocketException(
         "Could not set broadcast flag!", strerror(errno)));
   }
   
   return error == 0;
}
