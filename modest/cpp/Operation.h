/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_Operation_H
#define db_modest_Operation_H

#include "Thread.h"
#include "OperationGuard.h"
#include "StateMutator.h"

namespace db
{
namespace modest
{

// forward declare Engine and OperationExecutor
class Engine;
class OperationExecutor;

/**
 * An Operation is some set of instructions that can be executed by an
 * Engine provided that its current State is compatible with this Operation's
 * OperationGuard. An Operation may change the current State of the
 * Engine that executes it.
 * 
 * Operations running on the same Engine share its State information and can
 * therefore specify the conditions underwhich they can execute. This provides
 * developers with an easy yet powerful way to protect resources and restrict
 * the flow of their code such that it only executes in the specific manner
 * desired.
 * 
 * @author Dave Longley
 */
class Operation : public virtual db::rt::Object
{
protected:
   /**
    * The Runnable for this Operation.
    */
   db::rt::Runnable* mRunnable;
   
   /**
    * The guard that decides when this Operation can execute.
    */
   OperationGuard* mGuard;
   
   /**
    * The StateMutator for this Operation.
    */
   StateMutator* mStateMutator;
   
   /**
    * The Thread this Operation is executing on.
    */
   db::rt::Thread* mThread;
   
   /**
    * Set to true if this Operation has started, false otherwise.
    */
   bool mStarted;
   
   /**
    * Set to true if this Operation has been interrupted, false otherwise.
    */
   bool mInterrupted;
   
   /**
    * Set to true if this Operation finished, false otherwise.
    */
   bool mFinished;
   
   /**
    * Set to true if this Operation was canceled, false otherwise.
    */
   bool mCanceled;
   
   /**
    * Set to true once this Operation has stopped.
    */
   bool mStopped;
   
   /**
    * Set to true once this Operation is collectable.
    */
   bool mCollectable;
   
   /**
    * OperationExecutor is a friend so that it can manipulate Operations
    * without publically exposing protected Operation information.
    */
   friend class OperationExecutor;
   
public:
   /**
    * Creates a new Operation that can execute the given Runnable in the
    * given guard. The passed Runnable must be fully interruptible.
    * 
    * @param r the Runnable to execute.
    * @param g the OperationGuard underwhich this Operation can execute.
    * @param m the StateMutator for this Operation.
    */
   Operation(
      db::rt::Runnable* r, OperationGuard* g = NULL, StateMutator* m = NULL);
   
   /**
    * Destructs this Operation.
    */
   virtual ~Operation();
   
   /**
    * Waits for this Operation to finish or be canceled once it has been
    * executed by an Engine. This method is interruptible by default, meaning
    * the method can return before the Operation is finished or canceled if
    * the current thread is interrupted. If the Operation *must* be finished
    * or canceled before the current thread can continue, this method can
    * be made uninterruptible by passing false.
    * 
    * @param interruptible true if the current thread can be interrupted
    *                      and return from this call, false if the Operation
    *                      must complete before this call will return.
    * 
    * @return true if the current thread was interrupted while waiting, false
    *         if not.
    */
   virtual bool waitFor(bool interruptible = true);
   
   /**
    * Returns true if this Operation has started, false if not.
    */
   virtual bool started();
   
   /**
    * Interrupts this Operation. If this Operation is waiting to be
    * executed, it will be cancelled. If this Operation is already executing,
    * then a call to Operation::interrupted() will return true inside of
    * the Runnable for this Operation. A call to isInterrupted() will return
    * true from anywhere.
    * 
    * Operations that are interrupted must cease activity and exit gracefully.
    */
   virtual void interrupt();
   
   /**
    * Returns true if this Operation has been interrupted, false if not.
    * 
    * @return true if this Operation has been interrupted, false if not.
    */
   virtual bool isInterrupted();
   
   /**
    * Returns true if this Operation finished and was not canceled. Even
    * if this method returns true, this Operation's memory should not be
    * reclaimed until collectable() returns true.
    * 
    * @return true if this Operation finished and was not canceled,
    *         false otherwise.
    */
   virtual bool finished();
   
   /**
    * Returns true if this Operation was canceled and did not finish. Even
    * if this method returns true, this Operation's memory should not be
    * reclaimed until collectable() returns true. 
    * 
    * @return true if this Operation was canceled and did not finish,
    *         false otherwise.
    */
   virtual bool canceled();
   
   /**
    * Returns true if it is safe to reclaim this Operation's memory.
    * 
    * @return true if it is safe to reclaim this Operation's memory.
    */
   virtual bool collectable();
   
   /**
    * Gets the Runnable for this Operation.
    * 
    * @return the Runnable for this Operation.
    */
   virtual db::rt::Runnable* getRunnable();
   
   /**
    * Gets this Operation's guard.
    * 
    * @return this Operation's guard, which may be NULL.
    */
   virtual OperationGuard* getGuard();
   
   /**
    * Gets this Operation's StateMutator.
    * 
    * @return this Operation's StateMutator, which may be NULL.
    */
   virtual StateMutator* getStateMutator();
   
   /**
    * Returns true if the current Operation has been interrupted, false if not.
    * 
    * @return true if the current Operation has been interrupted, false if not.
    */
   static bool interrupted();
};

} // end namespace modest
} // end namespace db
#endif
