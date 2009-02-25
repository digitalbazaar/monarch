/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_DatagramServicer_H
#define db_net_DatagramServicer_H

#include "db/net/DatagramSocket.h"

namespace db
{
namespace net
{

/**
 * A DatagramServicer receives Datagrams from the passed DatagramSocket and
 * services them in some implementation specific fashion.
 * 
 * @author Dave Longley
 */
class DatagramServicer
{
public:
   /**
    * Creates a new DatagramServicer.
    */
   DatagramServicer() {};
   
   /**
    * Destructs this DatagramServicer.
    */
   virtual ~DatagramServicer() {};
   
   /**
    * Performs initialization work on the DatagramSocket once it is
    * bound, if any is necessary, and returns false if an exception occurs.
    * 
    * @param s the DatagramSocket to be serviced.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool initialize(DatagramSocket* s) { return true; };
   
   /**
    * Receives Datagrams from the passed DatagramSocket and services them.
    * 
    * @param s the DatagramSocket with Datagrams to service.
    */
   virtual void serviceDatagrams(DatagramSocket* s) = 0;
};

} // end namespace net
} // end namespace db
#endif
