/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef UdpSocket_H
#define UdpSocket_H

#include "AbstractSocket.h"

namespace db
{
namespace net
{

/**
 * A UdpSocket is a Socket that usesUDP datagrams.
 * 
 * @author Dave Longley
 */
class UdpSocket : public AbstractSocket
{
protected:
   /**
    * Acquiring a file descriptor for this Socket. This method must be called
    * before trying to use this Socket.
    * 
    * This method is called automatically by the default implementation.
    * 
    * @param domain the communication domain for this Socket (i.e. IPv4, IPv6).
    * 
    * @return true if the file descriptor could be acquired, false if
    *         an exception occurred.
    */
   virtual bool acquireFileDescriptor(const std::string& domain);
   
   /**
    * Creates a new Socket with the given file descriptor that points to
    * the socket for an accepted connection.
    * 
    * @param fd the file descriptor for the socket.
    * 
    * @return the allocated Socket.
    */
   virtual Socket* createConnectedSocket(unsigned int fd);
   
public:
   /**
    * Creates a new UdpSocket.
    */
   UdpSocket();
   
   /**
    * Destructs this UdpSocket.
    */
   virtual ~UdpSocket();
   
   /**
    * Joins a multicast group with the given address.
    * 
    * @param group the multicast group address.
    * @param localAddress the local address to bind to.
    * 
    * @return true if the join was successful, false if an exception occurred.
    */
   virtual bool joinGroup(
      SocketAddress* group, SocketAddress* localAddress = NULL);
   
   /**
    * Leaves a multicast group with the given address.
    * 
    * @param group the multicast group address.
    * 
    * @return true if the leave was successful, false if an exception occurred.
    */
   virtual bool leaveGroup(SocketAddress* group);
   
   /**
    * Writes a datagram to some SocketAddress.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * @param address the SocketAddres to write to.
    * 
    * @return true if the send was successful, false if an exception occurred.
    */
   virtual bool sendDatagram(
      const char* b, unsigned int length, SocketAddress* address);
   
   /**
    * Receives a datagram. This method will block until at least one datagram
    * can be read. The address the datagram is from will be written to the
    * passed SocketAddress, if it is not NULL.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * @param address the SocketAddress to populate.
    * 
    * @return the number of bytes read or -1 if an exception occurred.
    */
   virtual int receiveDatagram(
      char* b, unsigned int length, SocketAddress* address = NULL);
   
   /**
    * Sets the IPv6 multicast hops. This is the number of hops a datagram
    * should make before dying.
    * 
    * Note: This method is for IPv6 only.
    * 
    * @param hops the number of hops to use.
    * 
    * @return true if the hops were set, false if an exception occurred.
    */
   virtual bool setMulticastHops(unsigned char hops);
   
   /**
    * Sets the IPv4 multicast time-to-live (TTL). This is the number of hops a
    * datagram should make before dying.
    * 
    * Note: This method is for IPv4 only.
    * 
    * @param ttl the time-to-live to use.
    * 
    * @return true if the ttl was set, false if an exception occurred.
    */
   virtual bool setMulticastTimeToLive(unsigned char ttl);
   
   /**
    * Enables/disables broadcasting via this socket.
    * 
    * @param enable true to enable broadcasting, false to disable it.
    * 
    * @return true if broadcasting was set, false if an exception occurred.
    */
   virtual bool setBroadcastEnabled(bool enable);
};

} // end namespace net
} // end namespace db
#endif
