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

void Observable::dispatchEvent(Event e)
{
   // go through the list of EventId taps
   EventId id = e["id"]->getUInt64(); 
   EventIdMap::iterator ti = mTaps.find(id);
   if(ti != mTaps.end())
   {
      // create an operation list and vector for storing event dispatchers
      OperationList opList;
      EventDispatcherList edList;
      
      EventIdMap::iterator end = mTaps.upper_bound(id);
      for(; ti != end; ti++)
      {
         // go through the list of observers for the EventId tap
         ObserverMap::iterator oi = mObservers.find(ti->second);
         if(oi != mObservers.end())
         {
            ObserverMap::iterator oend = mObservers.upper_bound(ti->second);
            for(; oi != oend; oi++)
            {
               // create and run event dispatcher for each observable
               EventDispatcher* ed = new EventDispatcher(oi->second, &e);
               Operation op = mOpRunner->createOperation(ed, NULL, NULL);
               mOpRunner->runOperation(op);
               opList.add(op);
               edList.push_back(ed);
            }
         }
      }
      
      // unlock, wait for dispatch operations to complete, relock
      unlock();
      opList.waitFor();
      opList.prune();
      lock();
      
      // clean up event dispatchers
      for(EventDispatcherList::iterator i = edList.begin();
          i != edList.end(); i++)
      {
         delete *i;
      }
   }
}

void Observable::dispatchEvents()
{
   lock();
   {
      while(!mEventQueue.empty())
      {
         // dispatch the next event
         Event e = mEventQueue.front();
         mEventQueue.pop_front();
         dispatchEvent(e);
      }
      
      // turn off dispatching
      mDispatch = false;
   }
   unlock();
}

void Observable::registerObserver(Observer* observer, EventId id)
{
   lock();
   {
      // add tap to self if EventId doesn't exist yet
      EventIdMap::iterator i = mTaps.find(id);
      if(i == mTaps.end())
      {
         mTaps.insert(make_pair(id, id));
      }
      
      // add observer
      mObservers.insert(make_pair(id, observer));
   }
   unlock();
}

void Observable::unregisterObserver(Observer* observer, EventId id)
{
   lock();
   {
      ObserverMap::iterator i = mObservers.find(id);
      if(i != mObservers.end())
      {
         ObserverMap::iterator end = mObservers.upper_bound(id);
         for(; i != end; i++)
         {
            if(i->second == observer)
            {
               // remove observer and break
               mObservers.erase(i);
               break;
            }
         }
      }
   }
   unlock();
}

void Observable::addTap(EventId id, EventId tap)
{
   lock();
   {
      // add tap to id-self if EventId doesn't exist yet
      EventIdMap::iterator i = mTaps.find(id);
      if(i == mTaps.end())
      {
         mTaps.insert(make_pair(id, id));
      }
      
      // insert tap for id
      mTaps.insert(make_pair(id, tap));
      
      // add tap to tap-self if EventId doesn't exist yet
      i = mTaps.find(tap);
      if(i == mTaps.end())
      {
         mTaps.insert(make_pair(tap, tap));
      }
   }
   unlock();
}

void Observable::removeTap(EventId id, EventId tap)
{
   lock();
   {
      // look for tap in the range of taps
      EventIdMap::iterator i = mTaps.find(id);
      if(i != mTaps.end())
      {
         EventIdMap::iterator end = mTaps.upper_bound(id);
         for(; i != end; i++)
         {
            if(i->second == tap)
            {
               // remove tap and break
               mTaps.erase(i);
               break;
            }
         }
      }
   }
   unlock();
}

void Observable::schedule(Event e, EventId id, bool async)
{
   lock();
   {
      // set the event's ID
      e["id"] = id;
      
      if(async)
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
      else
      {
         // dispatch the event immediately
         dispatchEvent(e);
      }
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
