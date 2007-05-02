/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SocketAddress_H
#define SocketAddress_H

#include "SocketException.h"

#include <string>

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
   SocketAddress() : mAddress("0.0.0.0"), mPort(0) {};
   
   /**
    * Creates a new SocketAddress with the specified address and port.
    * 
    * @param address the address (or host).
    * @param port the socket port.
    */
   SocketAddress(std::string address, unsigned short port) :
      mAddress(address), mPort(port) {};
   
   /**
    * Destructs this SocketAddress.
    */
   virtual ~SocketAddress() {};   
   
   /**
    * Sets the address part of the socket address. 
    * 
    * @param address the address to use.
    * 
    * @exception SocketException thrown if the address is invalid.
    */
   virtual void setAddress(const std::string& address) throw(SocketException);
   
   /**
    * Gets the address part of the socket address. 
    * 
    * @return the address.
    */
   virtual std::string getAddress();
   
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

inline void SocketAddress::setAddress(const std::string& address)
throw(SocketException)
{
   mAddress = address;
}

inline std::string SocketAddress::getAddress()
{
   return mAddress;
}

inline void SocketAddress::setPort(unsigned short port)
{
   mPort = port;
}

inline unsigned short SocketAddress::getPort()
{
   return mPort;
}

} // end namespace net
} // end namespace db
#endif
