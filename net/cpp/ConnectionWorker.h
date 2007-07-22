/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_ConnectionWorker_H
#define db_net_ConnectionWorker_H

#include "Runnable.h"
#include "Connection.h"
#include "Operation.h"

namespace db
{
namespace net
{

// forward declare ConnectionService
class ConnectionService;

/**
 * A ConnectionWorker is a Runnable object that works for a ConnectionService
 * to get a Connection serviced.
 * 
 * @author Dave Longley
 */
class ConnectionWorker : public db::rt::Runnable
{
protected:
   /**
    * The ConnectionService this worker works for.
    */
   ConnectionService* mService;
   
   /**
    * The Connection to get serviced.
    */
   Connection* mConnection;
   
   /**
    * The Operation used to run this worker.
    */
   db::modest::Operation* mOperation;
   
public:
   /**
    * Creates a new ConnectionWorker that works for the passed ConnectionService
    * to get the passed Connection serviced.
    * 
    * @param s the ConnectionService.
    * @param c the Connection to get serviced.
    */
   ConnectionWorker(ConnectionService* s, Connection* c);
   
   /**
    * Destructs this ConnectionWorker.
    */
   virtual ~ConnectionWorker();
   
   /**
    * Gets the Connection serviced.
    */
   virtual void run();
   
   /**
    * Sets the Operation used to run this worker.
    * 
    * @param op the Operation used to run this worker.
    */
   virtual void setOperation(db::modest::Operation* op);
   
   /**
    * Gets the Operation used to run this worker.
    * 
    * @return the Operation used to run this worker.
    */
   virtual db::modest::Operation* getOperation();
};

} // end namespace net
} // end namespace db
#endif
