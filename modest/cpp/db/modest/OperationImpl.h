/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_OperationImpl_H
#define db_modest_OperationImpl_H

#include "db/rt/Thread.h"
#include "db/modest/OperationGuardChain.h"
#include "db/modest/StateMutatorChain.h"

namespace db
{
namespace modest
{

/**
 * An OperationImpl is the basic implementation for an Operation.
 * 
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
class OperationImpl : public virtual db::rt::Object, protected db::rt::Runnable
{
protected:
   /**
    * The Runnable for this Operation. A regular runnable or a
    * RunnableRef may be used.
    */
   db::rt::Runnable* mRunnable;
   db::rt::RunnableRef mRunnableReference;
   
   /**
    * The guard that decides when this Operation can execute.
    */
   CollectableOperationGuard mGuard;
   
   /**
    * The StateMutator for this Operation.
    */
   CollectableStateMutator mMutator;
   
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
    * Set to true once this Operation has stopped.
    */
   bool mStopped;
   
   /**
    * Set to true if this Operation finished, false otherwise.
    */
   bool mFinished;
   
   /**
    * Set to true if this Operation was canceled, false otherwise.
    */
   bool mCanceled;
   
   /**
    * OperationDispatcher is a friend to allow it run this Operation and
    * mark it as canceled if necessary.
    */
   friend class OperationDispatcher;
   
   /**
    * Stops this Operation.
    */
   virtual void stop();
   
   /**
    * Executes the Operation's Runnable on the current Thread. This method
    * should only be called by an Engine's OperationDispatcher.
    */
   virtual void run();
   
public:
   /**
    * Creates a new OperationImpl that can execute the given Runnable. The
    * passed Runnable must handle interruptions gracefully.
    * 
    * @param r the Runnable to execute.
    */
   OperationImpl(db::rt::Runnable& r);
   OperationImpl(db::rt::RunnableRef& r);
   
   /**
    * Destructs this OperationImpl.
    */
   virtual ~OperationImpl();
   
   /**
    * Waits for this Operation to stop once it has been queued by an Engine.
    * The Operation will be marked as finished or canceled.
    * 
    * This method is interruptible by default, meaning the method can return
    * before the Operation stops if the current thread is interrupted. If the
    * Operation *must* be stopped before the current thread can continue, this
    * method can be made uninterruptible by passing false (if the thread is
    * interrupted, this method will return it in an interrupted state after the
    * Operation has stopped).
    * 
    * @param interruptible true if the current thread can be interrupted
    *                      and return from this call, false if the Operation
    *                      must complete before this call will return.
    * 
    * @return false if the current thread was interrupted while waiting (with
    *         an InterruptedException set), true if it was not interrupted.
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
    * Returns true if the Engine that this Operation was queued with has
    * stopped the Operation.
    * 
    * @return true if this Operation has stopped, false if it is still in use.
    */
   virtual bool stopped();
   
   /**
    * Returns true if this Operation finished and was not canceled.
    * 
    * @return true if this Operation finished and was not canceled,
    *         false otherwise.
    */
   virtual bool finished();
   
   /**
    * Returns true if this Operation was canceled and did not finish.
    * 
    * @return true if this Operation was canceled and did not finish,
    *         false otherwise.
    */
   virtual bool canceled();
   
   /**
    * Gets the Runnable for this Operation.
    * 
    * @return the Runnable for this Operation.
    */
   virtual db::rt::Runnable* getRunnable();
   
   /**
    * Adds an OperationGuard to this Operation. This method must only be called
    * prior to running this Operation or undefined behavior will result.
    * 
    * @param g the OperationGuard to add.
    * @param front true to add this OperationGuard in front of any existing
    *              ones (the default behavior), false to add it to the back.
    */
   virtual void addGuard(OperationGuard* g, bool front = true);
   virtual void addGuard(CollectableOperationGuard& g, bool front = true);
   
   /**
    * Gets this Operation's first guard.
    * 
    * @return this Operation's first guard, which may be NULL.
    */
   virtual OperationGuard* getGuard();
   
   /**
    * Adds a StateMutator to this Operation. This method must only be called
    * prior to running this Operation or undefined behavior will result.
    * 
    * @param m the StateMutator to add.
    * @param front true to add this StateMutator in front of any existing
    *              ones (the default behavior), false to add it to the back.
    */
   virtual void addStateMutator(StateMutator* m, bool front = true);
   virtual void addStateMutator(CollectableStateMutator& m, bool front = true);
   
   /**
    * Gets this Operation's first StateMutator.
    * 
    * @return this Operation's first StateMutator, which may be NULL.
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
