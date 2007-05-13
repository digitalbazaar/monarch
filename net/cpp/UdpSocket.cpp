/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "UdpSocket.h"

using namespace db::io;
using namespace db::net;

UdpSocket::UdpSocket()
{
}

UdpSocket::~UdpSocket()
{
}

void UdpSocket::initialize() throw(SocketException)
{
   if(mFileDescriptor == -1)
   {
      create(SOCK_DGRAM, IPPROTO_UDP);
   }
}

Socket* UdpSocket::createConnectedSocket(unsigned int fd) throw(SocketException)
{
   // create a new UdpSocket
   UdpSocket* socket = new UdpSocket();
   socket->mFileDescriptor = fd;
   socket->mBound = true;
   socket->mConnected = true;
   
   return socket;
}

void UdpSocket::joinGroup(SocketAddress* group, SocketAddress* localAddress)
throw(SocketException)
{
   // create multicast request
   struct ip_mreq request;
   
   // set multicast address
   inet_aton(group->getAddress().c_str(), &request.imr_multiaddr);
   
   if(localAddress == NULL)
   {
      // use any address for local interface
      request.imr_interface.s_addr = INADDR_ANY;
   }
   else
   {
      // set local interface
      inet_aton(localAddress->getAddress().c_str(), &request.imr_interface);
   }
   
   int error = setsockopt(
      mFileDescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,
      (char*)&request, sizeof(request));
   if(error < 0)
   {
      throw SocketException("Could not join multicast group!", strerror(errno));
   }
}

void UdpSocket::leaveGroup(SocketAddress* group) throw(SocketException)
{
   // create multicast request
   struct ip_mreq request;
   
   // set multicast address
   inet_aton(group->getAddress().c_str(), &request.imr_multiaddr);
   
   // use any address for local interface
   request.imr_interface.s_addr = INADDR_ANY;
   
   int error = setsockopt(
      mFileDescriptor, IPPROTO_IP, IP_DROP_MEMBERSHIP,
      (char*)&request, sizeof(request));
   if(error < 0)
   {
      throw SocketException(
         "Could not leave multicast group!", strerror(errno));
   }
}

void UdpSocket::sendDatagram(
   const char* b, unsigned int length, SocketAddress* address)
throw(IOException)
{
   if(!isBound())
   {
      throw SocketException("Cannot write to unbound Socket!");
   }
   
   // create sockaddr_in (internet socket address) structure
   struct sockaddr_in addr;
   socklen_t addrSize = sizeof(addr);
   
   // populate address structure
   populateAddressStructure(address, addr);
   
   // send all data (send can fail to send all bytes in one go because the
   // socket send buffer was full)
   unsigned int offset = 0;
   while(length > 0)
   {
      // wait for socket to become writable
      select(false, getSendTimeout());
      
      int bytes = sendto(
         mFileDescriptor, b + offset, length, 0, (sockaddr*)&addr, addrSize);
      if(bytes < 0)
      {
         throw SocketException("Could not write to Socket!", strerror(errno));
      }
      else if(bytes > 0)
      {
         offset += bytes;
         length -= bytes;
      }
   }
}

int UdpSocket::receiveDatagram(
   char* b, unsigned int length, SocketAddress* address)
throw(IOException)
{
   int rval = 0;
   
   if(!isBound())
   {
      throw SocketException("Cannot read from unbound Socket!");
   }
   
   // wait for data to become available
   select(true, getReceiveTimeout());
   
   // create sockaddr_in (internet socket address) structure, if appropriate
   struct sockaddr_in addr;
   socklen_t addrSize = sizeof(addr);
   
   // receive some data
   rval = recvfrom(
      mFileDescriptor, b, length, 0, (sockaddr*)&addr, &addrSize);
   if(rval < -1)
   {
      throw SocketException("Could not read from Socket!", strerror(errno));
   }
   else if(rval != 0 && address != NULL)
   {
      // put address into SocketAddress
      address->setAddress(inet_ntoa(addr.sin_addr));
      
      // FIXME: handle converting from network byte order to little-endian
      address->setPort(addr.sin_port);
      //address->setPort(nstoh(addr.sin_port));
   }
   
   return rval;
}

void UdpSocket::setMulticastTimeToLive(unsigned char ttl) throw(SocketException)
{
   // set multicast flag
   int error = setsockopt(
      mFileDescriptor, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));
   if(error < 0)
   {
      throw SocketException("Could not set multicast TTL!", strerror(errno));
   }
}

void UdpSocket::setBroadcastEnabled(bool enable) throw(SocketException)
{
   // set broadcast flag
   int broadcast = (enable) ? 1 : 0;
   int error = setsockopt(
      mFileDescriptor, SOL_SOCKET, SO_BROADCAST,
      (char *)&broadcast, sizeof(broadcast));
   if(error < 0)
   {
      throw SocketException("Could not set broadcast flag!", strerror(errno));
   }
}
