/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef InternetAddress_H
#define InternetAddress_H

#include "SocketAddress.h"
#include "UnknownHostException.h"

namespace db
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
   std::string mHost;
   
public:
   /**
    * Creates a new InternetAddress.
    */
   InternetAddress();
   
   /**
    * Creates a new InternetAddress with the specified host and port.
    * 
    * @param host the host.
    * @param port the socket port.
    * 
    * @exception UnknownHostException thrown if the host cannot be resolved.
    */
   InternetAddress(const std::string& host, unsigned short port)
   throw(UnknownHostException);
   
   /**
    * Destructs this InternetAddress.
    */
   virtual ~InternetAddress();
   
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
    * Sets the address part of the socket address. 
    * 
    * @param address the address to use.
    */
   virtual void setAddress(const std::string& address);
   
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
};

} // end namespace net
} // end namespace db
#endif
