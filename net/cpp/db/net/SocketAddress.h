/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_SocketAddress_H
#define db_net_SocketAddress_H

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
public:
   enum CommunicationDomain
   {
      IPv4,
      IPv6
   };
   
protected:
   /**
    * The domain associated with this socket address.
    */
   CommunicationDomain mCommDomain;
   
   /**
    * The address part of the socket address.
    */
   char* mAddress;
   
   /**
    * The port part of the socket address.
    */
   unsigned short mPort;
   
public:
   /**
    * Creates a new SocketAddress with the specified address and port.
    * 
    * @param domain the communication domain.
    * @param address the address (or host).
    * @param port the socket port.
    */
   SocketAddress(
      CommunicationDomain domain = IPv4,
      const char* address = "0.0.0.0",
      unsigned short port = 0);
   
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
    * Sets the communication domain for the socket address, i.e. IPv4, IPv6.
    * 
    * @param domain the communication domain to use.
    */
   virtual void setCommunicationDomain(CommunicationDomain domain);
   
   /**
    * Gets the communication domain for the socket address, i.e. IPv4, IPv6. 
    * 
    * @return the communication domain.
    */
   virtual CommunicationDomain getCommunicationDomain();
   
   /**
    * Sets the address part of the socket address. 
    * 
    * @param address the address to use.
    */
   virtual void setAddress(const char* address);
   
   /**
    * Gets the address part of the socket address. 
    * 
    * @return the address.
    */
   virtual const char* getAddress();
   
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
   
   /**
    * Gets a string representation for this SocketAddress.
    * 
    * @param simple true for simple representation that can be converted
    *               back into a SocketAddress, false for complex
    *               representation for display only.
    * 
    * @return a string representation for this SocketAddress.
    */
   virtual std::string toString(bool simple = true);
   
   /**
    * Converts the passed string into this SocketAddress.
    * 
    * @param str the string to convert into this SocketAddress.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool fromString(const char* str);
   
   /**
    * Converts the passed communication domain into a string representation.
    * 
    * @param domain the communication domain.
    * 
    * @return the string representation.
    */
   static const char* communicationDomainToString(CommunicationDomain domain);
};

} // end namespace net
} // end namespace db
#endif
