/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
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
    * Sends a Datagram.
    *
    * @param datagram the Datagram to send.
    *
    * @return true if the Datagram was sent, false if an exception occurred.
    */
   virtual bool send(DatagramRef& datagram);

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
   virtual bool receive(DatagramRef& datagram);

   // use remainder of UdpSocket interface
   using UdpSocket::bind;
   using UdpSocket::joinGroup;
   using UdpSocket::leaveGroup;
   using UdpSocket::setMulticastHops;
   using UdpSocket::setMulticastTimeToLive;
   using UdpSocket::setBroadcastEnabled;
   using UdpSocket::close;
   using UdpSocket::isBound;
   using UdpSocket::isConnected;
   using UdpSocket::getLocalAddress;
   using UdpSocket::setSendTimeout;
   using UdpSocket::getSendTimeout;
   using UdpSocket::setReceiveTimeout;
   using UdpSocket::getReceiveTimeout;
};

} // end namespace net
} // end namespace db
#endif
