/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Operation_H
#define Operation_H

#include "Thread.h"
#include "Environment.h"

namespace db
{
namespace modest
{

/**
 * An Operation is some set of instructions that can be executed by an
 * Engine provided that it's current State is compatible with this Operation's
 * execution Environment. An Operation may change the current State of the
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
class Operation : public virtual db::rt::Object, public db::rt::Runnable
{
protected:
   /**
    * The Runnable for this Operation.
    */
   db::rt::Runnable* mRunnable;
   
   /**
    * The execution Environment for this Operation.
    */
   Environment* mExecutionEnvironment;
   
   /**
    * The Thread this Operation is executing on.
    */
   db::rt::Thread* mThread;
   
   /**
    * Set to true if this Operation has been interrupted, false otherwise.
    */
   bool mInterrupted;
   
public:
   /**
    * Creates a new Operation that can execute the given Runnable in the
    * given Environment.
    * 
    * @param r the Runnable to execute.
    * @param e the Environment underwhich the Runnable can execute.
    */
   Operation(db::rt::Runnable* r, Environment* e);
   
   /**
    * Destructs this Operation.
    */
   virtual ~Operation();
   
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
    * Returns true if the current Operation has been interrupted, false if not.
    * 
    * @return true if the current Operation has been interrupted, false if not.
    */
   static bool interrupted();
};

} // end namespace modest
} // end namespace db
#endif
