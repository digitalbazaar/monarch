/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_ConnectionGuard_H
#define db_net_ConnectionGuard_H

#include "OperationGuard.h"
#include "Server.h"

namespace db
{
namespace net
{

/**
 * A ConnectionGuard determines when a Connection can be handled.
 * 
 * @author Dave Longley
 */
class ConnectionGuard : public db::modest::OperationGuard
{
protected:
   /**
    * The Server associated with this guard.
    */
   Server* mServer;
   
   /**
    * The ConnectionHandler associated with this guard.
    */
   ConnectionHandler* mHandler;
   
public:
   /**
    * Creates a new ConnectionGuard for the given ConnectionHandler.
    * 
    * @param s the Server associated with this guard.
    * @param h the ConnectionHandler associated with this guard.
    */
   ConnectionGuard(Server* s, ConnectionHandler* h);
   
   /**
    * Destructs this ConnectionGuard.
    */
   virtual ~ConnectionGuard();
   
   /**
    * Returns true if the passed State meets the conditions of this guard
    * such that an Operation that requires this guard could be executed
    * immediately by an Engine with the given State.
    * 
    * @param s the ImmutableState to inspect.
    * 
    * @return true if an Engine with the given State could immediately execute
    *         an Operation with this guard.
    */
   virtual bool canExecuteOperation(db::modest::ImmutableState* s);
   
   /**
    * Returns true if the passed State cannot meet the conditions of this
    * guard such that an Operation that requires this guard must be
    * immediately canceled.
    * 
    * This method may return false when an Operation may not be able to be
    * executed immediately, but it needn't be canceled either, instead it
    * can be checked again later for execution or cancelation.
    * 
    * @param s the ImmutableState to inspect.
    * 
    * @return true if an Engine with the given State must cancel an Operation
    *         with this guard, false if not.
    */
   virtual bool mustCancelOperation(db::modest::ImmutableState* s);
};

} // end namespace net
} // end namespace db
#endif
