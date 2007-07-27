/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_OperationExecutor_H
#define db_modest_OperationExecutor_H

#include "State.h"
#include "OperationDispatcher.h"
#include "Runnable.h"

namespace db
{
namespace modest
{

// forward declare OperationDispatcher
class OperationDispatcher;

/**
 * An OperationExecutor performs Engine State mutation and executes an
 * Operation.
 * 
 * @author Dave Longley
 */
class OperationExecutor : public virtual db::rt::Object, public db::rt::Runnable
{
protected:
   /**
    * State the Engine State to mutate.
    */
   State* mState;
   
   /**
    * The Operation to execute.
    */
   Operation* mOperation;
   
   /**
    * The OperationDispatcher to use to obtain a thread to execute the
    * Operation on and to notify when this executor has expired.
    */
   OperationDispatcher* mDispatcher;
   
public:
   /**
    * Creates a new OperationExecutor that can safely mutate the passed
    * State and execute the passed Operation.
    * 
    * @param s the State to mutate.
    * @param op the Operation to execute.
    * @param od the OperationDispatcher to use to obtain a thread to execute
    *           the Operation on and to notify when this executor has expired.
    */
   OperationExecutor(State* s, Operation* op, OperationDispatcher* od);
   
   /**
    * Destructs this OperationExecutor.
    */
   virtual ~OperationExecutor();
   
   /**
    * Performs the Operation's pre-execution state mutation.
    */
   virtual void doPreExecutionStateMutation();
   
   /**
    * Executes the Operation's Runnable on the current Thread.
    */
   virtual void run();
   
   /**
    * Checks the Operation's guard restrictions to see if the Operation
    * can be executed, should wait, or should be canceled.
    * 
    * @return 0 if the Operation can be executed, 1 if it should wait, or 2
    *         if it should be canceled.
    */
   virtual int checkGuard();
   
   /**
    * Cleans up this executor once it has expired.
    */
   virtual void cleanup();
   
   /**
    * Gets a string representation for this Runnable.
    * 
    * @param str the string to populate.
    * 
    * @return a string representation for this Runnable.
    */
   virtual std::string& toString(std::string& str);
};

} // end namespace modest
} // end namespace db
#endif
