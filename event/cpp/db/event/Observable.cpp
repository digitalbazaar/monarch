/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/Observable.h"
#include "db/modest/OperationList.h"

using namespace std;
using namespace db::event;
using namespace db::modest;
using namespace db::rt;

Observable::Observable()
{
   // no dispatch operation yet
   mOperation = NULL;
   
   // no events to dispatch yet
   mDispatch = false;
}

Observable::~Observable()
{
   // ensure event dispatching is stopped
   stop();
}

void Observable::dispatchEvents()
{
   lock();
   {
      // create an OperationList
      OperationList opList;
      
      // create array of event dispatchers
      EventDispatcher eds[mObservers.size()];
      
      while(mEventQueue.size() > 0)
      {
         // get the next event
         Event e = mEventQueue.front();
         mEventQueue.pop_front();
         
         // create and run operations for each observable
         int count = 0;
         for(ObserverList::iterator i = mObservers.begin();
             i != mObservers.end(); i++, count++)
         {
            eds[count].observer = *i;
            eds[count].event = e;
            Operation op = mOpRunner->createOperation(&eds[count], NULL, NULL);
            mOpRunner->runOperation(op);
            opList.add(op);
         }
         
         // wait for dispatch operations to complete
         opList.waitFor();
         opList.prune();
      }
      
      // turn off dispatching
      mDispatch = false;
   }
   unlock();
}

void Observable::registerObserver(Observer* observer)
{
   lock();
   {
      mObservers.push_back(observer);
   }
   unlock();
}

void Observable::unregisterObserver(Observer* observer)
{
   lock();
   {
      mObservers.remove(observer);
   }
   unlock();
}

void Observable::schedule(Event e)
{
   lock();
   {
      // lock to set dispatch condition
      mDispatchLock.lock();
      mDispatch = true;
      
      // add event to event queue
      mEventQueue.push_back(e);
      
      // notify on dispatch lock and release
      mDispatchLock.notifyAll();
      mDispatchLock.unlock();
   }
   unlock();
}

void Observable::start(OperationRunner* opRunner)
{
   lock();
   {
      if(mOperation == NULL)
      {
         // store operation runner
         mOpRunner = opRunner;
         mOperation = opRunner->createOperation(this, NULL, NULL);
         opRunner->runOperation(mOperation);
      }
   }
   unlock();
}

void Observable::stop()
{
   lock();
   {
      if(mOperation != NULL)
      {
         mOperation->interrupt();
         mOperation->waitFor();
         mOperation = NULL;
      }
   }
   unlock();
}

void Observable::run()
{
   // keep dispatching until interrupted
   while(!mOperation->isInterrupted())
   {
      // lock dispatch lock and check to see if we can dispatch
      mDispatchLock.lock();
      if(mDispatch)
      {
         mDispatchLock.unlock();
         dispatchEvents();
      }
      else
      {
         // wait until dispatch condition is marked true and we are notified
         mDispatchLock.wait();
         mDispatchLock.unlock();
      }
   }
}
