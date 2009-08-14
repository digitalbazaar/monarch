/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/event/Observable.h"

#include "db/rt/DynamicObjectIterator.h"
#include "db/rt/Iterator.h"
#include "db/event/ObserverDelegate.h"
#include <algorithm>

using namespace std;
using namespace db::event;
using namespace db::modest;
using namespace db::rt;

#define MAX_SEQ_ID (uint64_t)0xffffffffffffffffULL

Observable::Observable() :
   mDispatch(false),
   mSequenceId(0)
{
}

Observable::~Observable()
{
   // ensure event dispatching is stopped
   Observable::stop();
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

/**
 * A helper function that will wait for an observer to finish processing
 * events before proceeding -- unless the thread that is processing the
 * events is the current thread. This prevents a race condition where an
 * observer that is to be unregistered could be concurrently processing
 * an event during the unregister call. If this wait is not performed to
 * allow the observer to finish event processing, then the observer could
 * be free'd whilst processing an event, resulting in a segfault.
 *
 * Note: This method assumes the registration lock is engaged when it is
 * called.
 *
 * @param lock the registration lock.
 * @param observer the observer being unregistered.
 * @param opList the operation list that may contain the observer's event
 *               processing operation(s).
 */
static void waitForObserver(
   ExclusiveLock& lock, Observer* observer, OperationList& opList)
{
   Thread* t = Thread::currentThread();
   bool mustWait;

   do
   {
      // must wait for the observer to finish event processing but only if
      // the unregistration request is not coming from this thread
      // (prevents deadlock)
      mustWait = false;
      OperationList tmpOpList;
      IteratorRef<Operation> itr = opList.getIterator();
      while(itr->hasNext())
      {
         Operation& op = itr->next();
         if(op->getUserData() == observer && t != op->getThread())
         {
            tmpOpList.add(op);
            mustWait = true;
         }
      }

      if(mustWait)
      {
         // unlock registration lock to allow other processing while waiting
         lock.unlock();

         // wait for observer event processing operations
         itr = tmpOpList.getIterator();
         while(itr->hasNext())
         {
            Operation& op = itr->next();

            // Note: It is possible right now for deadlock to occur if
            // two parallel event processing threads attempt to concurrently
            // unregister their observer... this is currently considered a
            // programmer error and is documented as such. However, we could
            // potentially fix this if we were to add a map of observer to
            // operation list where we store each operation that is trying to
            // unregister an observer ... and we allow at least one operation
            // from that list to proceed at a time.

            // wait for operation to complete
            op->waitFor();
         }
         opList.prune();

         // relock registration lock
         lock.lock();
      }
   }
   while(mustWait);
}

void Observable::unregisterObserver(Observer* observer, EventId id)
{
   mRegistrationLock.lock();
   {
      // wait for the observer to finish any event processing
      waitForObserver(mRegistrationLock, observer, mOpList);

      // find the filter map for the event
      ObserverMap::iterator i = mObservers.find(id);
      if(i != mObservers.end())
      {
         // remove observer
         vector<EventFilter> removableFilters;
         removeObserverFromFilterMap(observer, i->second, removableFilters);

         // erase filter map if it would be empty after processing
         // the removable filters
         if(removableFilters.size() == i->second.size())
         {
            mObservers.erase(i);
         }
         // remove all appropriate filters
         else
         {
            for(vector<EventFilter>::iterator fi = removableFilters.begin();
                fi != removableFilters.end(); fi++)
            {
               i->second.erase(*fi);
            }
         }
      }
   }
   mRegistrationLock.unlock();
}

void Observable::unregisterObserver(Observer* observer)
{
   // FIXME: add an optimization that stores all of the events that the
   // observer is registered to receive to shorten this unregistration process

   mRegistrationLock.lock();
   {
      // wait for the observer to finish any event processing
      waitForObserver(mRegistrationLock, observer, mOpList);

      // iterate over all filter maps, keep a list of event IDs to remove
      vector<EventId> removeIds;
      for(ObserverMap::iterator i = mObservers.begin();
          i != mObservers.end(); i++)
      {
         // remove observer
         vector<EventFilter> removableFilters;
         removeObserverFromFilterMap(observer, i->second, removableFilters);

         // mark filter map for removal if it would be empty after processing
         // the removable filters
         if(removableFilters.size() == i->second.size())
         {
            removeIds.push_back(i->first);
         }
         // remove all appropriate filters
         else
         {
            for(vector<EventFilter>::iterator fi = removableFilters.begin();
                fi != removableFilters.end(); fi++)
            {
               i->second.erase(*fi);
            }
         }
      }

      // remove all appropriate filter map entries
      for(std::vector<EventId>::iterator ei = removeIds.begin();
          ei != removeIds.end(); ei++)
      {
         mObservers.erase(*ei);
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

      // set event's sequence ID
      mSequenceId = (mSequenceId == MAX_SEQ_ID) ? 1 : mSequenceId + 1;
      e["sequenceId"] = mSequenceId;

      // add event to queue
      mEventQueue.push_back(e);

      // notify on queue lock and release
      mQueueLock.notifyAll();
      mQueueLock.unlock();
   }
   else
   {
      // set event's sequence ID
      mQueueLock.lock();
      mSequenceId = (mSequenceId == MAX_SEQ_ID) ? 1 : mSequenceId + 1;
      e["sequenceId"] = mSequenceId;
      mQueueLock.unlock();

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

void Observable::removeObserverFromFilterMap(
   Observer* observer, FilterMap& fm,
   vector<EventFilter>& removableFilters)
{
   // remove the observer from every filter's list
   for(FilterMap::iterator fi = fm.begin(); fi != fm.end(); fi++)
   {
      // erase all instances of the observer in this filter's list
      for(ObserverList::iterator li = fi->second.begin();
          li != fi->second.end();)
      {
         if(*li == observer)
         {
            // erase observer entry
            li = fi->second.erase(li);
         }
         else
         {
            li++;
         }
      }

      // if filter's list is now empty, mark it for removal
      if(fi->second.empty())
      {
         removableFilters.push_back(fi->first);
      }
   }
}

void Observable::dispatchEvent(
   Event& e, EventId id, OperationList& waitList)
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
                     for(ObserverList::iterator li =
                         fi->second.begin(); li != fi->second.end(); li++)
                     {
                        // create and run event dispatcher for observer
                        // set operation user data to observer
                        RunnableRef ed = new ObserverDelegate<Observer>(*li, e);
                        Operation op(ed);
                        op->setUserData(*li);
                        mOpRunner->runOperation(op);

                        // add all operations to the current operation list
                        mOpList.add(op);

                        // only add serial events to the wait list,
                        // parallel events are not waited on for completion
                        if(!e->hasMember("parallel") ||
                           !e["parallel"]->getBoolean())
                        {
                           waitList.add(op);
                        }
                     }
                  }
               }
            }
         }
         else
         {
            // dispatch event to tap
            dispatchEvent(e, ti->second, waitList);
         }
      }
   }
}

void Observable::dispatchEvent(Event& e)
{
   // create an operation list for waiting
   OperationList waitList;

   // lock to process event and prevent registration modification
   mRegistrationLock.lock();

   // get the EventId for the event and dispatch it
   EventId id = e["id"]->getUInt64();
   dispatchEvent(e, id, waitList);

   if(!waitList.isEmpty())
   {
      // unlock registration and wait for dispatch operations to complete
      mRegistrationLock.unlock();

      if(!waitList.waitFor())
      {
         // dispatch thread interrupted, so interrupt all
         // event dispatches and wait for them to complete
         waitList.interrupt();
         waitList.waitFor(false);
         mOpList.interrupt();
         mOpList.waitFor(false);
      }

      // relock registration
      mRegistrationLock.lock();
   }

   // prune operation list
   mOpList.prune();

   // unlock registration lock
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
