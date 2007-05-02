/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef InternetAddress_H
#define InternetAddress_H

#include <string>

#include "SocketAddress.h"

namespace db
{
namespace net
{

/**
 * An InternetAddress represents an address that uses the Internet Protocol.
 * 
 * It is made up of a IP address and a port or a hostname (which resolves
 * to an IP address) and a port.
 * 
 * This class is the base class for IP4 and IP6 addresses.
 * 
 * @author Dave Longley
 */
class InternetAddress : public virtual SocketAddress
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
    * Creates a new InternetAddress with the specified address and port.
    * 
    * @param address the address (or host).
    * @param port the socket port.
    */
   InternetAddress(const std::string& address, unsigned short port);
   
   /**
    * Destructs this InternetAddress.
    */
   virtual ~InternetAddress();
   
   /**
    * Sets the address part of the socket address. 
    * 
    * @param address the address to use.
    * 
    * @exception SocketException thrown if the address is invalid.
    */
   virtual void setAddress(const std::string& address) throw(SocketException);
   
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
