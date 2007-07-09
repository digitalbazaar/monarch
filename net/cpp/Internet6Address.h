/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_Internet6Address_H
#define db_net_Internet6Address_H

#include "InternetAddress.h"
#include "UnknownHostException.h"

namespace db
{
namespace net
{

/**
 * An Internet6Address represents an address that uses the Internet Protocol.
 * 
 * It is made up of a IP address and a port or a hostname (which resolves
 * to an IP address) and a port.
 * 
 * This class is the base class for IP4 and IP6 addresses.
 * 
 * @author Dave Longley
 */
class Internet6Address : public InternetAddress
{
public:
   /**
    * Creates a new Internet6Address.
    */
   Internet6Address();
   
   /**
    * Creates a new Internet6Address with the specified host and port. An
    * exception may be raised if the host is unknown.
    *
    * @param host the host.
    * @param port the socket port.
    */
   Internet6Address(const std::string& host, unsigned short port);
   
   /**
    * Destructs this Internet6Address.
    */
   virtual ~Internet6Address();
   
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
    * Sets the hostname for this address.
    * 
    * @param host the hostname for this address.
    * 
    * @return an UnknownHostException if the host is not known, NULL otherwise.
    */
   virtual UnknownHostException* setHost(const std::string& host);
   
   /**
    * Gets the hostname for this address.
    * 
    * @return the hostname for this address.
    */
   virtual const std::string& getHost();
   
   /**
    * Returns true if this address is a multicast address, false if not.
    * 
    * @return true if this address is a multicast address, false if not.
    */
   virtual bool isMulticast();
};

} // end namespace net
} // end namespace db
#endif
