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
    * Services the passed Connection. When finished, cleanupConnection() must
    * be called on the passed ConnectionService, which will ensure the that
    * Connection is closed and its memory cleaned up.
    * 
    * @param c the Connection to service.
    * @param cs the ConnectionService the Connection is from.
    */
   virtual void serviceConnection(Connection* c, ConnectionService* cs) = 0;
};

} // end namespace net
} // end namespace db
#endif
