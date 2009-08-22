/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_Datagram_H
#define db_net_Datagram_H

#include "db/io/ByteBuffer.h"
#include "db/net/InternetAddress.h"
#include "db/rt/Collectable.h"

namespace db
{
namespace net
{

/**
 * A Datagram is a self-contained, independent entity of data that can
 * be transported over the Internet.
 *
 * @author Dave Longley
 */
class Datagram
{
protected:
   /**
    * The source or destination address for this datagram.
    */
   InternetAddressRef mAddress;

   /**
    * The data for this datagram.
    */
   db::io::ByteBuffer mBuffer;

public:
   /**
    * Creates a new Datagram with no yet specified outgoing or incoming
    * InternetAddress. A call to setAddress() must be made before attempting
    * to send or receive this Datagram.
    *
    * If a capacity is specified, the internal data buffer will automatically
    * be allocated to the specified amount. Otherwise, it can be allocated
    * manually via getBuffer().
    *
    * @param capacity the maximum capacity for the data in this datagram, 0
    *                 to set it manually via getBuffer().
    */
   Datagram(int capacity = 0);

   /**
    * Creates a new Datagram. An InternetAddress must be specified that will
    * either be used to send this datagram or it will be populated when this
    * datagram is used to receive data.
    *
    * If a capacity is specified, the internal data buffer will automatically
    * be allocated to the specified amount. Otherwise, it can be allocated
    * manually via getBuffer().
    *
    * @param address the InternetAddress to associate with this datagram.
    * @param capacity the maximum capacity for the data in this datagram, 0
    *                 to set it manually via getBuffer().
    */
   Datagram(InternetAddressRef& address, int capacity = 0);

   /**
    * Destructs this Datagram.
    */
   virtual ~Datagram();

   /**
    * Sets the address for this datagram. This is either the source or
    * destination address for this datagram.
    *
    * @param address the address for this datagram.
    */
   virtual void setAddress(InternetAddressRef& address);

   /**
    * Gets the address for this datagram. This is either the source or
    * destination address for this datagram.
    *
    * @return the address for this datagram.
    */
   virtual InternetAddressRef& getAddress();

   /**
    * Gets the data buffer for this datagram. It may be modified as the
    * user of this datagram sees fit.
    *
    * When a datagram is received, this buffer will be cleared and filled
    * with the data for the datagram, with no more bytes than its capacity.
    * Therefore, getBuffer()->resize(<desired_capacity>) should be called
    * before receiving this datagram.
    *
    * When a datagram is sent, the buffer's data, starting at its offset and
    * up to its length, will be sent. The buffer will not be cleared.
    *
    * @return the data buffer for this datagram.
    */
   virtual db::io::ByteBuffer* getBuffer();

   /**
    * Assigns the data for this datagram to the passed string. The
    * terminating NULL character will NOT be included. The internal
    * buffer for this datagram will become managed if it is not already.
    *
    * @param str the string to use.
    */
   virtual void assignString(const std::string& str);

   /**
    * Gets this datagram's data as a string.
    *
    * @return this datagram's data as a string.
    */
   virtual std::string getString();
};

// typedef for a reference counted Datagram
typedef db::rt::Collectable<Datagram> DatagramRef;

} // end namespace net
} // end namespace db
#endif
