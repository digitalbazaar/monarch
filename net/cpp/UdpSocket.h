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
    * Initializes this Socket by acquiring a file descriptor for it. This
    * method must be called before trying to use this TcpSocket.
    * 
    * @exception SocketException thrown if the Socket could not be initialized.
    */
   virtual void initialize() throw(SocketException);
   
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
    * Receives a datagram. This method will block until at least one datagram
    * can be read. The address the datagram is from will be written to the
    * passed SocketAddress, if it is not NULL.
    * 
    * @param b the array of bytes to fill.
    * @param offset the offset at which to start filling the array.
    * @param length the maximum number of bytes to read into the buffer.
    * @param address the SocketAddress to populate.
    * 
    * @return the number of bytes read.
    * 
    * @exception SocketException thrown if a socket error occurs. 
    */
   virtual int receiveDatagram(
      char* b, int offset, int length, SocketAddress* address = NULL)
   throw(SocketException);
   
   /**
    * Writes a datagram to some SocketAddress.
    * 
    * @param b the array of bytes to write.
    * @param offset the offset at which to start reading from the array.
    * @param length the number of bytes to write to the stream.
    * @param address the SocketAddres to write to.
    * 
    * @exception SocketException thrown if a socket error occurs. 
    */
   virtual void sendDatagram(
      char* b, int offset, int length, SocketAddress* address)
   throw(SocketException);
   
   /**
    * Sets the multicast time-to-live (TTL). This is the number of hops a
    * datagram should make before dying. 
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
