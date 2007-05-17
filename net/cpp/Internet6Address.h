/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Internet6Address_H
#define Internet6Address_H

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
    * Creates a new Internet6Address with the specified host and port.
    * 
    * @param host the host.
    * @param port the socket port.
    * 
    * @exception UnknownHostException thrown if the host cannot be resolved.
    */
   Internet6Address(const std::string& host, unsigned short port)
   throw(UnknownHostException);
   
   /**
    * Destructs this Internet6Address.
    */
   virtual ~Internet6Address();
   
   /**
    * Converts this address to a sockaddr structure.
    * 
    * @param addr the sockaddr structure to populate.
    */
   virtual void toSockAddr(sockaddr* addr);
   
   /**
    * Converts this address from a sockaddr structure.
    * 
    * @param addr the sockaddr structure convert from.
    */
   virtual void fromSockAddr(const sockaddr* addr);
   
   /**
    * Sets the hostname for this address.
    * 
    * @return the hostname for this address.
    */
   virtual void setHost(const std::string& host) throw(UnknownHostException);
   
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
