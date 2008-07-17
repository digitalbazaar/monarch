/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_ConnectionServicer_H
#define db_net_ConnectionServicer_H

#include "db/net/ConnectionService.h"

namespace db
{
namespace net
{

/**
 * A ConnectionServicer services a Connection in some implementation specific
 * fashion.
 * 
 * @author Dave Longley
 */
class ConnectionServicer
{
public:
   /**
    * Creates a new ConnectionServicer.
    */
   ConnectionServicer() {};
   
   /**
    * Destructs this ConnectionServicer.
    */
   virtual ~ConnectionServicer() {};
   
   /**
    * Services the passed Connection. This method should end by closing the
    * passed Connection. After this method returns, the Connection will be
    * automatically closed, if necessary, and cleaned up.
    * 
    * @param c the Connection to service.
    */
   virtual void serviceConnection(Connection* c) = 0;
};

} // end namespace net
} // end namespace db
#endif
