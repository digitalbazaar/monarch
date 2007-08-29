/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_DatagramSocket_H
#define db_net_DatagramSocket_H

#include "db/net/Datagram.h"
#include "db/net/UdpSocket.h"

namespace db
{
namespace net
{

/**
 * A DatagramSocket provides a communication interface for transmitting and
 * receiving Datagrams.
 * 
 * @author Dave Longley
 */
class DatagramSocket : protected UdpSocket
{
public:
   /**
    * Creates a new DatagramSocket.
    */
   DatagramSocket();
   
   /**
    * Destructs this DatagramSocket.
    */
   virtual ~DatagramSocket();
   
   /**
    * Binds this Socket to the passed address.
    * 
    * @param address the address to bind to.
    * 
    * @return true if bound, false if an exception occurred.
    */
   virtual bool bind(InternetAddress* address);   
   
   /**
    * Joins a multicast group with the given address.
    * 
    * @param group the multicast group address.
    * @param localAddress the local address to bind to.
    * 
    * @return true if the join was successful, false if an exception occurred.
    */
   virtual bool joinGroup(
      InternetAddress* group, InternetAddress* localAddress = NULL);
   
   /**
    * Leaves a multicast group with the given address.
    * 
    * @param group the multicast group address.
    * 
    * @return true if the leave was successful, false if an exception occurred.
    */
   virtual bool leaveGroup(InternetAddress* group);
   
   /**
    * Sends a Datagram.
    * 
    * @param datagram the Datagram to send.
    * 
    * @return true if the Datagram was sent, false if an exception occurred. 
    */
   virtual bool send(Datagram* datagram);
   
   /**
    * Receives a datagram. This method will block until at least one Datagram
    * can be read. The address the Datagram is from and its data will be
    * written to the passed Datagram.
    * 
    * If the passed Datagram does not have a data buffer large enough to
    * accommodate the received data, then it will be truncated.
    * 
    * @param datagram the Datagram to populate.
    * 
    * @return true if a Datagram was received, false if an exception occurred. 
    */
   virtual bool receive(Datagram* datagram);
   
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
   
   /**
    * Closes this Socket. This will be done automatically when the Socket is
    * destructed.
    */
   virtual void close();
   
   /**
    * Returns true if this Socket is bound, false if not.
    * 
    * @return true if this Socket is bound, false if not.
    */
   virtual bool isBound();
   
   /**
    * Returns true if this Socket is connected, false if not.
    * 
    * @return true if this Socket is connected, false if not.
    */
   virtual bool isConnected();
   
   /**
    * Gets the local address for this Socket.
    * 
    * @param address the address to populate.
    * 
    * @return true if the address was populated, false if an exception occurred.
    */
   virtual bool getLocalAddress(InternetAddress* address);
   
   /**
    * Sets the send timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to send data.
    * 
    * @param timeout the send timeout in milliseconds.
    */
   virtual void setSendTimeout(unsigned long timeout);
   
   /**
    * Gets the send timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to send data.
    * 
    * @return the send timeout in milliseconds.
    */
   virtual unsigned long getSendTimeout();
   
   /**
    * Sets the receive timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to receive data.
    * 
    * @param timeout the receive timeout in milliseconds.
    */
   virtual void setReceiveTimeout(unsigned long timeout);
   
   /**
    * Gets the receive timeout for this Socket. This is the amount of time that
    * this Socket will block waiting to receive data.
    * 
    * @return the receive timeout in milliseconds.
    */
   virtual unsigned long getReceiveTimeout();
};

} // end namespace net
} // end namespace db
#endif
