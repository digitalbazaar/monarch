/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_DatagramHandler_H
#define db_net_DatagramHandler_H

#include "Datagram.h"
#include "InternetAddress.h"

namespace db
{
namespace net
{

/**
 * A DatagramHandler receives incoming Datagrams and passes them off to a
 * DatagramServicer.
 * 
 * @author Dave Longley
 */
class DatagramHandler : public virtual db::rt::Object
{
protected:
   /**
    * The address to bind to.
    */
   InternetAddress* mAddress;
   
   /**
    * True to clean up the internet address when destructing, false not to.
    */
   bool mCleanup;
   
public:
   /**
    * Creates a new DatagramHandler that uses the passed address for
    * communication.
    * 
    * @param address the address to listen on.
    * @param cleanup true to clean up the internet address when destructing,
    *                false not to.
    */
   DatagramHandler(InternetAddress* address, bool cleanup);
   
   /**
    * Destructs this DatagramHandler.
    */
   virtual ~DatagramHandler();
   
   /**
    * Gets the address for this DatagramHandler.
    * 
    * @return the address for this DatagramHandler.
    */
   virtual InternetAddress* getAddress();
};

} // end namespace net
} // end namespace db
#endif
