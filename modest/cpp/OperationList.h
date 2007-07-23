/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_OperationList_H
#define db_modest_OperationList_H

#include "Operation.h"

#include <list>

namespace db
{
namespace modest
{

/**
 * An OperationList is used to keep track of multiple Operations. It can
 * be used to interrupt all Operations in the list at once. When this list is
 * destructed, all of its Operations will be automatically interrupted, waited
 * for, and pruned.
 * 
 * @author Dave Longley
 */
class OperationList : public virtual db::rt::Object
{
protected:
   /**
    * The Operations in this list.
    */
   std::list<Operation*> mOperations;
   
   /**
    * True if this OperationList should free the memory associated with
    * the expired Operations in this list, false if not.
    */
   bool mCleanup;
   
public:
   /**
    * Creates a new OperationList. It may be set to either free the memory
    * associated with any expired Operations in this list or it can be
    * set to leave them alone.
    * 
    * @param cleanup true if this list should assume responsibility for
    *                cleaning up its Operations.
    */
   OperationList(bool cleanup);
   
   /**
    * Destructs this OperationList. All Operations in this list will be
    * interrupted and pruned.
    */
   virtual ~OperationList();
   
   /**
    * Adds the passed Operation to this list.
    * 
    * @param op the Operation to add.
    */
   virtual void add(Operation* op);
   
   /**
    * Interrupts all Operations in this list.
    */
   virtual void interrupt();
   
   /**
    * Waits for every single Operation in this list to finish or be canceled.
    * 
    * This method is interruptible by default, meaning the method can return
    * before all Operations are finished or canceled if the current thread is
    * interrupted. If all Operations *must* be finished or canceled before the
    * current thread can continue, this method can be made uninterruptible by
    * passing false.
    * 
    * @param interruptible true if the current thread can be interrupted
    *                      and return from this call, false if all the
    *                      Operations must complete before this call will
    *                      return.
    */
   virtual void waitFor(bool interruptible = true);
   
   /**
    * Checks for expired Operations and removes them from this list. If this
    * list was set to clean up Operations, the memory associated with any
    * expired Operations will be freed before they are removed from this list.
    */
   virtual void prune();
   
   /**
    * Interrupts, waits for, and prunes all Operations in this list. This
    * method will not return until every Operation in this list has terminated,
    * even if the current thread is interrupted.
    */
   virtual void terminate();
};

} // end namespace modest
} // end namespace db
#endif
