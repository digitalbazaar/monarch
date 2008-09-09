/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/Observable.h"

#include "db/rt/DynamicObjectIterator.h"

#include <algorithm>

using namespace std;
using namespace db::event;
using namespace db::modest;
using namespace db::rt;

Observable::Observable()
{
   // no events to dispatch yet
   mDispatch = false;
}

Observable::~Observable()
{
   // ensure event dispatching is stopped
   stop();
}

void Observable::dispatchEvent(
   Event& e, EventId id, OperationList& opList)
{
   // go through the list of EventId taps
   EventIdMap::iterator ti = mTaps.find(id);
   if(ti != mTaps.end())
   {
      EventIdMap::iterator end = mTaps.upper_bound(id);
      for(; ti != end; ti++)
      {
         // dispatch event if the tap is the EventId itself
         if(ti->second == id)
         {
            // get the filter map for the EventId tap
            ObserverMap::iterator oi = mObservers.find(id);
            if(oi != mObservers.end())
            {
               FilterMap& fm = oi->second;
               
               // go through each filter
               bool pass;
               for(FilterMap::iterator fi = fm.begin(); fi != fm.end(); fi++)
               {
                  // check observer filter if appropriate
                  pass = true;
                  if(!fi->first.isNull())
                  {
                     // filter must be a subset of event
                     pass = fi->first.isSubset(e);
                  }
                  
                  if(pass)
                  {
                     // create and run event dispatcher for each observer
                     for(ObserverList::iterator li =
                         fi->second.begin(); li != fi->second.end(); li++)
                     {
                        RunnableRef ed = new EventDispatcher(*li, &e);
                        Operation op(ed);
                        mOpRunner->runOperation(op);
                        
                        // only add serial events to the operation list,
                        // parallel events are not waited on for completion
                        if(!e->hasMember("parallel") ||
                           !e["parallel"]->getBoolean())
                        {
                           opList.add(op);
                        }
                     }
                  }
               }
            }
         }
         else
         {
            // dispatch event to tap
            dispatchEvent(e, ti->second, opList);
         }
      }
   }
}

void Observable::dispatchEvent(Event& e)
{
   // create an operation list
   OperationList opList;
   
   // lock to process event and prevent registration modification
   mRegistrationLock.lock();
   {
      // get the EventId for the event and dispatch it
      EventId id = e["id"]->getUInt64();
      dispatchEvent(e, id, opList);
      
      if(!opList.isEmpty())
      {
         // wait for dispatch operations to complete
         if(!opList.waitFor())
         {
            // dispatch thread interrupted, so interrupt all
            // event dispatches and wait for them to complete
            opList.interrupt();
            opList.waitFor(false);
         }
      }
   }
   mRegistrationLock.unlock();
}

void Observable::dispatchEvents()
{
   // lock event queue
   mQueueLock.lock();
   
   // continue dispatching until no events or interrupted
   while(!mEventQueue.empty() && !mOperation->isInterrupted())
   {
      // get the next event
      Event e = mEventQueue.front();
      mEventQueue.pop_front();
      
      // unlock queue, dispatch event, relock queue      
      mQueueLock.unlock();
      dispatchEvent(e);
      mQueueLock.lock();
   }
   
   // turn off dispatching
   mDispatch = false;
   
   // unlock event queue
   mQueueLock.unlock();
}

void Observable::registerObserver(
   Observer* observer, EventId id, DynamicObject* filter)
{
   mRegistrationLock.lock();
   {
      // add tap to self if EventId doesn't exist yet
      EventIdMap::iterator i = mTaps.find(id);
      if(i == mTaps.end())
      {
         mTaps.insert(make_pair(id, id));
      }
      
      // get the event ID's filter map, creating it if necessary
      ObserverMap::iterator oi = mObservers.find(id);
      if(oi == mObservers.end())
      {
         // add a filter map
         FilterMap tmp;
         pair<ObserverMap::iterator, bool> p =
            mObservers.insert(make_pair(id, tmp));
         oi = p.first;
      }
      
      // create the event filter to compare against
      EventFilter ef(NULL);
      if(filter != NULL)
      {
         ef = filter->clone();
      }
      
      // get the observer list, creating it if necessary
      FilterMap::iterator fi = oi->second.find(ef);
      if(fi == oi->second.end())
      {
         // add an observer list
         ObserverList tmp;
         pair<FilterMap::iterator, bool> p =
            oi->second.insert(make_pair(ef, tmp));
         fi = p.first;
      }
      
      // add the observer to the list
      fi->second.push_back(observer);
   }
   mRegistrationLock.unlock();
}

void Observable::unregisterObserver(Observer* observer, EventId id)
{
   mRegistrationLock.lock();
   {
      // find the filter map for the event
      ObserverMap::iterator i = mObservers.find(id);
      if(i != mObservers.end())
      {
         // remove the observer from every list
         for(FilterMap::iterator fi = i->second.begin(); fi != i->second.end();)
         {
            ObserverList::iterator li = find(
               fi->second.begin(), fi->second.end(), observer);
            if(li != fi->second.end())
            {
               // erase observer from list
               fi->second.erase(li);
               
               // erase filter entry if observer list is empty
               if(fi->second.empty())
               {
                  FilterMap::iterator tmp = fi;
                  fi++;
                  i->second.erase(tmp);
                  
                  // erase filter map entry if event ID has no more observers
                  if(i->second.empty())
                  {
                     mObservers.erase(i);
                  }
               }
            }
            else
            {
               fi++;
            }
         }
      }
   }
   mRegistrationLock.unlock();
}

void Observable::addTap(EventId id, EventId tap)
{
   mRegistrationLock.lock();
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
   mRegistrationLock.unlock();
}

void Observable::removeTap(EventId id, EventId tap)
{
   mRegistrationLock.lock();
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
   mRegistrationLock.unlock();
}

void Observable::schedule(Event e, EventId id, bool async)
{
   // set the event's ID
   e["id"] = id;
   
   if(async)
   {
      // lock to modify event queue and dispatch condition
      mQueueLock.lock();
      mDispatch = true;
      
      // add event to queue
      mEventQueue.push_back(e);
      
      // notify on queue lock and release
      mQueueLock.notifyAll();
      mQueueLock.unlock();
   }
   else
   {
      // dispatch the event immediately
      dispatchEvent(e);
   }
}

void Observable::start(OperationRunner* opRunner)
{
   mRegistrationLock.lock();
   {
      if(mOperation.isNull())
      {
         // store operation runner, activate dispatching,
         // create and run operation
         mOpRunner = opRunner;
         mDispatch = true;
         mOperation = *this;
         opRunner->runOperation(mOperation);
      }
   }
   mRegistrationLock.unlock();
}

void Observable::stop()
{
   mRegistrationLock.lock();
   {
      if(!mOperation.isNull())
      {
         // interrupt dispatch operation
         mOperation->interrupt();
         
         // Note: We only care about locking in this method to prevent
         // start() from running while we are shutting down -- we don't
         // care if more events are scheduled because that won't cause
         // any conflicts. Since this is the case, and start() will
         // check mOperation.isNull() before starting anything, then
         // we don't need to worry about it starting here since mOperation
         // can't be NULL until we relock and clear it below.
         // 
         // We need to unlock() while we wait for the dispatch operation
         // to complete because it needs to lock after dispatching each
         // event and won't finish if we are holding the lock here waiting
         // for it to finish.
         
         // unlock, wait for dispatch operation to finish, relock
         mRegistrationLock.unlock();
         mOperation->waitFor();
         mRegistrationLock.lock();
         
         // clean up operation
         mOperation.setNull();
      }
   }
   mRegistrationLock.unlock();
}

void Observable::run()
{
   // keep dispatching until interrupted
   while(!mOperation->isInterrupted())
   {
      // lock event queue and check to see if we can dispatch
      mQueueLock.lock();
      if(mDispatch)
      {
         mQueueLock.unlock();
         dispatchEvents();
      }
      else
      {
         // wait until dispatch condition is marked true and we are notified
         mQueueLock.wait();
         mQueueLock.unlock();
      }
   }
}
