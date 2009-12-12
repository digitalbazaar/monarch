/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/UdpSocket.h"

#include "monarch/io/PeekInputStream.h"
#include "monarch/net/WindowsSupport.h"
#include "monarch/net/SocketInputStream.h"
#include "monarch/net/SocketOutputStream.h"
#include "monarch/rt/Exception.h"

#include <cstring>

using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;

UdpSocket::UdpSocket()
{
}

UdpSocket::~UdpSocket()
{
}

bool UdpSocket::acquireFileDescriptor(SocketAddress::CommunicationDomain domain)
{
   bool rval = true;

   if(mFileDescriptor == -1)
   {
      // use PF_INET = "protocol family internet" (which just so happens to
      // have the same value as AF_INET but that's only because different
      // protocols were never used with the same address family
      if(domain == SocketAddress::IPv6)
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

   if(group->getCommunicationDomain() == SocketAddress::IPv6)
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
      ExceptionRef e = new Exception(
         "Could not join multicast group.", SOCKET_EXCEPTION_TYPE);
      e->getDetails()["error"] = strerror(errno);
      Exception::set(e);
   }

   return error == 0;
}

bool UdpSocket::leaveGroup(SocketAddress* group)
{
   int error = 0;

   if(group->getCommunicationDomain() == SocketAddress::IPv6)
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
      ExceptionRef e = new Exception(
         "Could not leave multicast group.", SOCKET_EXCEPTION_TYPE);
      e->getDetails()["error"] = strerror(errno);
      Exception::set(e);
   }

   return error == 0;
}

bool UdpSocket::sendDatagram(const char* b, int length, SocketAddress* address)
{
   bool rval = true;

   if(!isBound())
   {
      ExceptionRef e = new Exception(
         "Cannot write to unbound socket.", SOCKET_EXCEPTION_TYPE);
      Exception::set(e);
      rval = false;
   }
   else
   {
      // populate address structure
      unsigned int size = 130;
      char addr[size];
      address->toSockAddr((sockaddr*)&addr, size);

      // send all data (sendto cannot fail to send all bytes in one go because
      // the socket send buffer was full, it will block until it has enough
      // room to send)
      // wait for socket to become writable
      if((rval = select(false, getSendTimeout())))
      {
         int ret = SOCKET_MACRO_sendto(
            mFileDescriptor, b, length, 0, (sockaddr*)&addr, size);
         if(ret < 0)
         {
            ExceptionRef e = new Exception(
               "Could not write to socket.", SOCKET_EXCEPTION_TYPE);
            e->getDetails()["error"] = strerror(errno);
            Exception::set(e);
            rval = false;
         }
      }
   }

   return rval;
}

int UdpSocket::receiveDatagram(char* b, int length, SocketAddress* address)
{
   int rval = -1;

   if(!isBound())
   {
      ExceptionRef e = new Exception(
         "Cannot read from unbound socket.", SOCKET_EXCEPTION_TYPE);
      Exception::set(e);
   }
   else if(select(true, getReceiveTimeout()))
   {
      // get address structure
      socklen_t size = 130;
      char addr[size];

      // receive some data
      rval = SOCKET_MACRO_recvfrom(
         mFileDescriptor, b, length, 0, (sockaddr*)&addr, &size);
      if(rval < -1)
      {
         rval = -1;
         ExceptionRef e = new Exception(
            "Could not read from socket.", SOCKET_EXCEPTION_TYPE);
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
      }
      else if(address != NULL)
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
      ExceptionRef e = new Exception(
         "Could not set multicast hops.", SOCKET_EXCEPTION_TYPE);
      e->getDetails()["error"] = strerror(errno);
      Exception::set(e);
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
      ExceptionRef e = new Exception(
         "Could not set multicast TTL.", SOCKET_EXCEPTION_TYPE);
      e->getDetails()["error"] = strerror(errno);
      Exception::set(e);
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
      ExceptionRef e = new Exception(
         "Could not set broadcast flag.", SOCKET_EXCEPTION_TYPE);
      e->getDetails()["error"] = strerror(errno);
      Exception::set(e);
   }

   return error == 0;
}
