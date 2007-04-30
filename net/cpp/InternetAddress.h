/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef InternetAddress_H
#define InternetAddress_H

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
public:
   /**
    * Creates a new InternetAddress.
    */
   InternetAddress();
   
   /**
    * Creates a new InternetAddress.
    */
   ~InternetAddress();
};

} // end namespace net
} // end namespace db
#endif
