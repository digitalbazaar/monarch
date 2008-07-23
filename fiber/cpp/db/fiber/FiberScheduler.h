/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_fiber_FiberScheduler_H
#define db_fiber_FiberScheduler_H

#include "db/modest/OperationList.h"
#include "db/modest/OperationRunner.h"
#include "db/fiber/Fiber.h"

#include <map>

namespace db
{
namespace fiber
{

/**
 * A FiberScheduler schedules and runs Fibers. It uses N modest Operations to
 * run however many Fibers are assigned to it. Each Operation shares the same
 * FiberScheduler, calling it to acquire the next scheduled Fiber to run each
 * time it finishes running a Fiber. 
 * 
 * @author Dave Longley
 */
class FiberScheduler : public db::rt::Runnable
{
protected:
   /**
    * Typedef and a map of FiberIds to Fibers.
    */
   typedef std::map<FiberId, Fiber*> FiberMap;
   FiberMap mFiberMap;
   
   /**
    * Typedef and a list of Fibers.
    */
   typedef std::list<Fiber*> FiberList;
   FiberList mFiberList;
   
   /**
    * An iterator for scheduling Fibers.
    */
   FiberList::iterator mFiberItr;
   
   /**
    * The list of Operations to run Fibers on.
    */
   db::modest::OperationList mOpList;
   
   /**
    * A list of available FiberIds from exited fibers.
    */
   typedef std::list<FiberId> FiberIdFreeList;
   FiberIdFreeList mFiberIdFreeList;
   
   /**
    * A FiberMessage is a message sent to a particular Fiber. It contains
    * the recipient Fiber's ID, and either a new state for the Fiber or
    * message data with the state set to Fiber::None.
    */
   struct FiberMessage
   {
      FiberId id;
      Fiber::State state;
      db::rt::DynamicObject* data;
   };
   
   /**
    * Typedef, system message queue and custom message queue.
    */
   typedef std::list<FiberMessage> MessageQueue;
   MessageQueue* mSystemMessages;
   MessageQueue* mCustomMessages;
   
   /**
    * An exclusive lock for scheduling the next Fiber.
    */
   db::rt::ExclusiveLock mScheduleLock;
   
   /**
    * An exclusive lock for swapping message queues.
    */
   db::rt::ExclusiveLock mMessageQueueLock;
   
   /**
    * An exclusive lock for waiting for work.
    */
   db::rt::ExclusiveLock mWorkWaitLock;
   
   /**
    * An exclusive lock for waiting for the fiber list to empty.
    */
   db::rt::ExclusiveLock mNoFibersWaitLock;
   
   /**
    * Called to notify operations that work (messages or fibers) is available.
    */
   virtual void workAvailable();
   
   /**
    * Called to cause the current operation to wait for work to become
    * available (messages or fibers).
    */
   virtual void waitForWork();
   
   /**
    * Queues the passed message for processing.
    * 
    * @param fm the message to queue.
    */
   virtual void queueMessage(FiberMessage& fm);
   
   /**
    * Sends a message to change the state of a Fiber.
    * 
    * @param id the FiberId of the Fiber to send the message to.
    * @param state the new state for the Fiber.
    */
   virtual void sendStateMessage(FiberId id, Fiber::State state);
   
   /**
    * Processes pending custom messages.
    */
   virtual void processCustomMessages();
   
   /**
    * Processes pending system messages and returns true if work is available.
    * 
    * @return true if work is available for fiber operations, false if not.
    */
   virtual bool processSystemMessages();
   
   /**
    * Increments the internal iterator to the next fiber.
    */
   virtual void nextFiber();
   
   /**
    * Removes the fiber pointed to by the internal iterator and
    * increments the iterator.
    */
   virtual void removeFiber();
   
   /**
    * Processes pending FiberMessages and runs the next scheduled Fiber. If
    * there is no Fiber to run, the current thread will wait until one becomes
    * available unless yield is true.
    * 
    * @param yield true if the current Fiber is yielding, false if not.
    */
   virtual void runNextFiber(bool yield);
   
public:
   /**
    * Creates a new FiberScheduler.
    */
   FiberScheduler();
   
   /**
    * Destructs this FiberScheduler.
    */
   virtual ~FiberScheduler();
   
   /**
    * Starts this FiberScheduler. It will create "numOps" Operations using
    * the passed OperationRunner to run its Fibers on.
    * 
    * @param opRunner the OperationRunner to use.
    * @param numOps the number of Operations to run Fibers on.
    */
   virtual void start(db::modest::OperationRunner* opRunner, int numOps);
   
   /**
    * Stops this FiberScheduler. This method will not cause its Fibers to
    * exit, they will just no longer run.
    */
   virtual void stop();
   
   /**
    * Stops this FiberScheduler once all fibers have exited. This method will
    * block the current thread until all fibers have exited.
    * 
    * @return true if all fibers exited, false if interrupted.
    */
   virtual bool stopOnLastFiberExit();
   
   /**
    * Adds a new Fiber to this FiberScheduler. The passed Fiber must be
    * heap-allocated. It will be deleted by this FiberScheduler when it
    * exits.
    * 
    * @param fiber the Fiber to add.
    * 
    * @return the FiberId assigned to the Fiber.
    */
   virtual FiberId addFiber(Fiber* fiber);
   
   /**
    * Sends a message to be processed the next time this scheduler is run.
    * 
    * @param id the FiberId of the Fiber to send the message to.
    * @param msg the message to send.
    */
   virtual void sendMessage(FiberId id, db::rt::DynamicObject& msg);
   
   /**
    * Yields the *current* Fiber. This *must* be called by a running Fiber.
    * 
    * @param id the ID of the running Fiber to yield.
    */
   virtual void yield(FiberId id);
   
   /**
    * Exits (terminates) any Fiber.
    * 
    * @param id the FiberId of the Fiber to exit.
    */
   virtual void exit(FiberId id);
   
   /**
    * Puts to sleep any non-exiting Fiber.
    * 
    * @param id the FiberId of the Fiber to put to sleep.
    */
   virtual void sleep(FiberId id);
   
   /**
    * Wakes up any sleeping Fiber.
    * 
    * @param id the FiberId of the Fiber to wakeup.
    */
   virtual void wakeup(FiberId id);
   
   /**
    * Runs this FiberScheduler. This method is executed inside of N modest
    * Operations.
    */
   virtual void run();
};

} // end namespace fiber
} // end namespace db
#endif
