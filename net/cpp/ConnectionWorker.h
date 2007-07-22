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

// forward declare ConnectionHandler
class ConnectionHandler;

/**
 * A ConnectionWorker is a Runnable object that works for a ConnectionHandler
 * to get a Connection serviced.
 * 
 * @author Dave Longley
 */
class ConnectionWorker : public db::rt::Runnable
{
protected:
   /**
    * The ConnectionHandler this worker works for.
    */
   ConnectionHandler* mHandler;
   
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
    * Creates a new ConnectionWorker that works for the passed ConnectionHandler
    * to get the passed Connection serviced.
    * 
    * @param h the ConnectionHandler.
    * @param c the Connection to get serviced.
    */
   ConnectionWorker(ConnectionHandler* s, Connection* c);
   
   /**
    * Destructs this ConnectionWorker.
    */
   virtual ~ConnectionWorker();
   
   /**
    * Gets the Connection serviced.
    */
   virtual void run();
   
   /**
    * Gets the Connection.
    * 
    * @return the Connection.
    */
   virtual Connection* getConnection();
   
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
