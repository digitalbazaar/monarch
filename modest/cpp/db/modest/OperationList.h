/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_OperationList_H
#define db_modest_OperationList_H

#include "db/modest/OperationRunner.h"

#include <list>

namespace db
{
namespace modest
{

/**
 * An OperationList is used to keep track of multiple Operations. It can
 * be used to perform an action on all Operations in the list at once, such
 * as queuing or interrupting. When this list is destructed, all of its
 * Operations will be automatically interrupted, waited for, and pruned.
 * 
 * @author Dave Longley
 */
class OperationList : public virtual db::rt::Object
{
protected:
   /**
    * The Operations in this list.
    */
   std::list<Operation> mOperations;
   
public:
   /**
    * Creates a new OperationList.
    */
   OperationList();
   
   /**
    * Destructs this OperationList. All Operations in this list will be
    * terminated (interrupted, waited for, and pruned).
    */
   virtual ~OperationList();
   
   /**
    * Returns an Operation in the list based on its index. The index
    * is assumed to be within the range of the list.
    * 
    * @param index the index of the Operation.
    * 
    * @return the Operation.
    */
   virtual Operation& operator[](int index);
   
   /**
    * Adds the passed Operation to this list.
    * 
    * @param op the Operation to add.
    */
   virtual void add(Operation& op);
   
   /**
    * Removes the passed Operation from this list.
    * 
    * @param op the Operation to remove.
    */
   virtual void remove(Operation& op);
   
   /**
    * Queues all of the Operations in this list with the given OperationRunner.
    * 
    * @param opRunner the OperationRunner to queue the Operations with.
    */
   virtual void queue(OperationRunner* opRunner);
   
   /**
    * Interrupts all Operations in this list.
    */
   virtual void interrupt();
   
   /**
    * Waits for every single Operation in this list to stop. Each Operation
    * will be marked as finished or canceled.
    * 
    * This method is interruptible by default, meaning the method can return
    * before all Operations are stopped if the current thread is interrupted.
    * If all Operations *must* be stopped before the current thread can
    * continue, this method can be made uninterruptible by passing false (if
    * the thread is interrupted, this method will return it in an interrupted
    * state after all Operations have stopped).
    * 
    * @param interruptible true if the current thread can be interrupted
    *                      and return from this call, false if all the
    *                      Operations must complete before this call will
    *                      return.
    * 
    * @return an InterruptedException if the current thread was interrupted
    *         while waiting, NULL if it was not interrupted.
    */
   virtual db::rt::InterruptedException* waitFor(bool interruptible = true);
   
   /**
    * Checks for expired Operations and removes them from this list.
    */
   virtual void prune();
   
   /**
    * Interrupts, waits for, and prunes all Operations in this list. This
    * method will not return until every Operation in this list has terminated,
    * even if the current thread is interrupted.
    */
   virtual void terminate();
   
   /**
    * Returns true if there are no Operations in this list, false if there
    * are.
    * 
    * @return true if this OperationList is empty, false if not.
    */
   virtual bool isEmpty();
   
   /**
    * Clears this list of all of its Operations.
    */
   virtual void clear();
   
   /**
    * Returns the number of Operations in this list.
    * 
    * @return the number of Operations in this list.
    */
   virtual bool length();
};

} // end namespace modest
} // end namespace db
#endif
