/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_InternetAddress_H
#define monarch_net_InternetAddress_H

#include "monarch/net/SocketAddress.h"
#include "monarch/rt/Collectable.h"

namespace monarch
{
namespace net
{

/**
 * An InternetAddress represents an address that uses the Internet Protocol.
 *
 * It is made up of a IPv4 address and a port or a hostname (which resolves
 * to an IP address) and a port.
 *
 * This class is the base class for IPv4 and IPv6 addresses.
 *
 * @author Dave Longley
 */
class InternetAddress : public SocketAddress
{
protected:
   /**
    * The hostname.
    */
   char* mHost;

public:
   /**
    * Creates a new InternetAddress with the specified host and port. An
    * exception may be raised if the host is unknown.
    *
    * @param host the host.
    * @param port the socket port.
    */
   InternetAddress(const char* host = "", unsigned short port = 0);

   /**
    * Destructs this InternetAddress.
    */
   virtual ~InternetAddress();

   /**
    * Converts this address to a sockaddr structure. The passed structure
    * must be large enough to accommodate the address or this method
    * will fail.
    *
    * @param addr the sockaddr structure to populate.
    * @param size the size of the passed structure which will be updated to
    *             the number of bytes used in the structure upon completion.
    *
    * @return true if the sockaddr was populated, false if not.
    */
   virtual bool toSockAddr(sockaddr* addr, unsigned int& size);

   /**
    * Converts this address from a sockaddr structure. The passed structure
    * must be large enough to contain the address or this method will fail.
    *
    * @param addr the sockaddr structure convert from.
    * @param size the size of the sockaddr structure to convert from.
    *
    * @return true if converted, false if not.
    */
   virtual bool fromSockAddr(const sockaddr* addr, unsigned int size);

   /**
    * Sets the address part of the socket address.
    *
    * @param address the address to use.
    */
   virtual void setAddress(const char* address);

   /**
    * Sets the hostname for this address.
    *
    * @param host the hostname for this address.
    *
    * @return true if the host resolved, false if an UnknownHost exception
    *         occurred.
    */
   virtual bool setHost(const char* host);

   /**
    * Gets the hostname for this address.
    *
    * @return the hostname for this address.
    */
   virtual const char* getHost();

   /**
    * Returns true if this address is a multicast address, false if not.
    *
    * @return true if this address is a multicast address, false if not.
    */
   virtual bool isMulticast();

   /**
    * Gets a string representation for this InternetAddress.
    *
    * @param simple true for simple representation that can be converted
    *               back into a InternetAddress, false for complex
    *               representation for display only.
    *
    * @return a string representation for this InternetAddress.
    */
   virtual std::string toString(bool simple = true);
};

// typedef for reference counted InternetAddress
typedef monarch::rt::Collectable<InternetAddress> InternetAddressRef;

} // end namespace net
} // end namespace monarch
#endif
