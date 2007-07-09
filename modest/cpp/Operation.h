/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Operation_H
#define Operation_H

#include "Thread.h"
#include "OperationEnvironment.h"
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
 * OperationEnvironment. An Operation may change the current State of the
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
    * The environment required for this Operation to execute.
    */
   OperationEnvironment* mEnvironment;
   
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
    * OperationExecutor is a friend so that it can manipulate Operations
    * without publically exposing protected Operation information.
    */
   friend class OperationExecutor;
   
public:
   /**
    * Creates a new Operation that can execute the given Runnable in the
    * given environment. The passed Runnable must be fully interruptible.
    * 
    * @param r the Runnable to execute.
    * @param e the environment underwhich this Operation can execute.
    * @param m the StateMutator for this Operation.
    */
   Operation(
      db::rt::Runnable* r,
      OperationEnvironment* e = NULL,
      StateMutator* m = NULL);
   
   /**
    * Destructs this Operation.
    */
   virtual ~Operation();
   
   /**
    * Waits for this Operation to finish or be canceled once it has been
    * executed by an Engine.
    */
   virtual void waitFor();
   
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
    * Gets this Operation's required environment.
    * 
    * @return this Operation's required environment, which may be NULL.
    */
   virtual OperationEnvironment* getEnvironment();
   
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
