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
    * @exception SocketException thrown if a file descriptor could not be
    *            acquired.
    */
   virtual void acquireFileDescriptor(const std::string& domain)
   throw(SocketException);
   
   /**
    * Creates a new Socket with the given file descriptor that points to
    * the socket for an accepted connection.
    * 
    * @param fd the file descriptor for the socket.
    * 
    * @return the allocated Socket.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual Socket* createConnectedSocket(unsigned int fd)
   throw(SocketException);   
   
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
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual void joinGroup(
      SocketAddress* group, SocketAddress* localAddress = NULL)
   throw(SocketException);
   
   /**
    * Leaves a multicast group with the given address.
    * 
    * @param group the multicast group address.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual void leaveGroup(SocketAddress* group) throw(SocketException);
   
   /**
    * Writes a datagram to some SocketAddress.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * @param address the SocketAddres to write to.
    * 
    * @exception IOException thrown if an IO error occurs. 
    */
   virtual void sendDatagram(
      const char* b, unsigned int length, SocketAddress* address)
   throw(db::io::IOException);
   
   /**
    * Receives a datagram. This method will block until at least one datagram
    * can be read. The address the datagram is from will be written to the
    * passed SocketAddress, if it is not NULL.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * @param address the SocketAddress to populate.
    * 
    * @return the number of bytes read.
    * 
    * @exception IOException thrown if an IO error occurs. 
    */
   virtual int receiveDatagram(
      char* b, unsigned int length, SocketAddress* address = NULL)
   throw(db::io::IOException);
   
   /**
    * Sets the IPv6 multicast hops. This is the number of hops a datagram
    * should make before dying.
    * 
    * Note: This method is for IPv6 only.
    * 
    * @param hops the number of hops to use.
    * 
    * @exception SocketException if a socket error occurs.
    */
   virtual void setMulticastHops(unsigned char hops) throw(SocketException);
   
   /**
    * Sets the IPv4 multicast time-to-live (TTL). This is the number of hops a
    * datagram should make before dying.
    * 
    * Note: This method is for IPv4 only.
    * 
    * @param ttl the time-to-live to use.
    * 
    * @exception SocketException if a socket error occurs.
    */
   virtual void setMulticastTimeToLive(unsigned char ttl)
   throw(SocketException);
   
   /**
    * Enables/disables broadcasting via this socket.
    * 
    * @param enable true to enable broadcasting, false to disable it.
    * 
    * @exception SocketException if a socket error occurs.
    */
   virtual void setBroadcastEnabled(bool enable) throw(SocketException);
};

} // end namespace net
} // end namespace db
#endif
