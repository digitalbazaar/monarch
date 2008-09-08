/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/EventDaemon.h"

#include "db/util/Timer.h"

using namespace std;
using namespace db::event;
using namespace db::modest;
using namespace db::rt;
using namespace db::util;

EventDaemon::EventData::EventData(Event& e, uint32_t i, int c) :
   event(e.clone())
{
   interval = remaining = i;
   count = c;
}

EventDaemon::EventDaemon()
{
   mRunning = false;
   mStartWaitTime = 0;
   mScheduleEvents = true;
}

EventDaemon::~EventDaemon()
{
   EventDaemon::stop();
}

void EventDaemon::reset()
{
   // lock to clear all events and notify
   mLock.lock();
   {
      mEvents.clear();
      mLock.notifyAll();
   }
   mLock.unlock();
}

void EventDaemon::start(OperationRunner* opRunner, EventController* ec)
{
   // lock to start event daemon
   mLock.lock();
   {
      if(!mRunning)
      {
         // store event controller
         mEventController = ec;
         
         // daemon should try to schedule events
         mScheduleEvents = true;
         
         // run daemon on an operation
         mOperation = *this;
         opRunner->runOperation(mOperation);
         mRunning = true;
      }
   }
   mLock.unlock();
}

void EventDaemon::stop()
{
   // lock to stop event daemon
   mLock.lock();
   {
      if(mRunning)
      {
         // interrupt daemon
         mOperation->interrupt();
         
         // wait for daemon to finish
         mLock.unlock();
         mOperation->waitFor();
         mLock.lock();
         
         // no longer running
         mRunning = false;
      }
   }
   mLock.unlock();
}

void EventDaemon::add(Event& e, uint32_t interval, int count)
{
   if(count == -1 || count > 0)
   {
      // lock to add an event
      mLock.lock();
      {
         // create event data and add to the list
         EventData ed(e, interval, count);
         mEvents.push_back(ed);
         
         // notify daemon to try to schedule events
         mScheduleEvents = true;
         mLock.notifyAll();
      }
      mLock.unlock();
   }
}

void EventDaemon::remove(const char* type)
{
   // lock to remove an event
   mLock.lock();
   {
      // find all events of the given type
      for(EventList::iterator i = mEvents.begin(); i != mEvents.end();)
      {
         if(strcmp(i->event["type"]->getString(), type) == 0)
         {
            // event found, remove it
            i = mEvents.erase(i);
         }
         else
         {
            // increment iterator
            i++;
         }
      }
      
      // notify daemon to try to schedule events
      mScheduleEvents = true;
      mLock.notifyAll();
   }
   mLock.unlock();
}

void EventDaemon::remove(Event& e)
{
   // lock to remove an event
   mLock.lock();
   {
      // find the event
      bool found = false;
      for(EventList::iterator i = mEvents.begin();
          !found && i != mEvents.end(); i++)
      {
         if(i->event == e)
         {
            // event found, remove it
            found = true;
            mEvents.erase(i);
         }
      }
      
      // notify daemon to try to schedule events
      mScheduleEvents = true;
      mLock.notifyAll();
   }
   mLock.unlock();
}

void EventDaemon::remove(Event& e, uint32_t interval)
{
   // lock to remove an event
   mLock.lock();
   {
      // find the event
      bool found = false;
      for(EventList::iterator i = mEvents.begin();
          !found && i != mEvents.end(); i++)
      {
         if(i->event == e && i->interval == interval)
         {
            // event found, remove it
            found = true;
            mEvents.erase(i);
         }
      }
      
      // notify daemon to try to schedule events
      mScheduleEvents = true;
      mLock.notifyAll();
   }
   mLock.unlock();
}

void EventDaemon::run()
{
   while(!mOperation->isInterrupted())
   {
      // lock to schedule events
      mLock.lock();
      
      if(mEvents.empty())
      {
         // wait until notified or interrupted
         mStartWaitTime = 0;
         mLock.wait();
      }
      else
      {
         // FIXME: this algorithm may be optimized such that every
         // iteration does not require passing over the entire loop --
         // this can be done by sorting according to time remaining
         // until the next event is scheduled
         
         // get the amount of time that has been waited, restart wait time
         uint32_t waited = (mStartWaitTime == 0 ?
            0 : Timer::getMilliseconds(mStartWaitTime));
         mStartWaitTime = Timer::startTiming();
         
         // iterate over event heap, scheduling all events that have waited
         // for their intervals, removing all non-repetitious events, and
         // keeping track of the new wait time
         uint32_t waitTime = 0;
         for(EventList::iterator i = mEvents.begin(); i != mEvents.end();)
         {
            // remove the event if count has reached 0
            if(i->count == 0)
            {
               i = mEvents.erase(i);
            }
            else
            {
               // if remaining time is less than or equal to waiting time
               if(i->remaining <= waited)
               {
                  // schedule event, reset remaining time
                  mEventController->schedule(i->event);
                  i->remaining = i->interval;
                  
                  // decrement count as appropriate
                  if(i->count > 0)
                  {
                     i->count--;
                  }
               }
               else
               {
                  // update remaining time
                  i->remaining -= waitTime;
               }
               
               // update wait time
               if(waitTime == 0 || i->remaining < waitTime)
               {
                  waitTime = i->remaining;
               }
               
               // increment iterator
               i++;
            }
         }
         
         // turn off scheduling events to try to wait for whole wait time
         mScheduleEvents = false;
         
         // wait if appropriate
         if(waitTime > 0)
         {
            // shave off time spent in loop
            waited = Timer::getMilliseconds(mStartWaitTime);
            waitTime = (waited >= waitTime ? 0 : waitTime - waited);
            if(waitTime > 0)
            {
               mStartWaitTime += waited;
               mLock.wait(waitTime, &mScheduleEvents, true);
            }
         }
      }
      
      // unlock to allow daemon modification
      mLock.unlock();
   }
}
