/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SocketAddress_H
#define SocketAddress_H

#include <string>

// forward declare socket address
struct sockaddr;

namespace db
{
namespace net
{

/**
 * A SocketAddress represents an address that a Socket can use for binding,
 * connecting, etc. This class is the base class for other socket addresses.
 * 
 * @author Dave Longley
 */
class SocketAddress
{
protected:
   /**
    * The protocol associated with this socket address.
    */
   std::string mProtocol;
   
   /**
    * The address part of the socket address.
    */
   std::string mAddress;
   
   /**
    * The port part of the socket address.
    */
   unsigned short mPort;
   
public:
   /**
    * Creates a new SocketAddress.
    */
   SocketAddress();
   
   /**
    * Creates a new SocketAddress with the specified address and port.
    * 
    * @param protocol the protocol.
    * @param address the address (or host).
    * @param port the socket port.
    */
   SocketAddress(
      const std::string& protocol,
      const std::string& address, unsigned short port);
   
   /**
    * Destructs this SocketAddress.
    */
   virtual ~SocketAddress();
   
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
   virtual bool toSockAddr(sockaddr* addr, unsigned int& size) = 0;
   
   /**
    * Converts this address from a sockaddr structure. The passed structure
    * must be large enough to contain the address or this method will fail.
    * 
    * @param addr the sockaddr structure convert from.
    * @param size the size of the sockaddr structure to convert from.
    * 
    * @return true if converted, false if not.
    */
   virtual bool fromSockAddr(const sockaddr* addr, unsigned int size) = 0;
   
   /**
    * Sets the protocol for the socket address. 
    * 
    * @param protocol the protocol to use.
    */
   virtual void setProtocol(const std::string& protocol);
   
   /**
    * Gets the protocol for the socket address. 
    * 
    * @return the protocol.
    */
   virtual const std::string& getProtocol();
   
   /**
    * Sets the address part of the socket address. 
    * 
    * @param address the address to use.
    */
   virtual void setAddress(const std::string& address);
   
   /**
    * Gets the address part of the socket address. 
    * 
    * @return the address.
    */
   virtual const std::string& getAddress();
   
   /**
    * Sets the port part of the socket address.
    * 
    * @param port the port to use.
    */
   virtual void setPort(unsigned short port);
   
   /**
    * Gets the port part of the socket address.
    * 
    * @return the port.
    */
   virtual unsigned short getPort();
};

} // end namespace net
} // end namespace db
#endif
