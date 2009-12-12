/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/event/EventWaiter.h"

#include "monarch/rt/DynamicObjectIterator.h"

#include <cstdlib>

using namespace std;
using namespace db::event;
using namespace db::rt;

EventWaiter::EventWaiter(EventController* ec)
{
   mEventController = ec;
   mEventTypes->setType(Array);
   reset();
}

EventWaiter::~EventWaiter()
{
   EventWaiter::stop();
}

void EventWaiter::reset()
{
   mEventOccurred = false;
   mEvents.clear();
}

void EventWaiter::start(const char* event, DynamicObject* condition)
{
   mEventTypes->append() = event;
   mEventController->registerObserver(this, event, condition);
}

void EventWaiter::fire()
{
   // fire off the first event
   Event e;
   e["type"] = mEventTypes[0]->getString();
   mEventController->schedule(e);
}

void EventWaiter::stop(const char* event)
{
   if(event == NULL)
   {
      // unregister all events
      DynamicObjectIterator i = mEventTypes.getIterator();
      while(i->hasNext())
      {
         DynamicObject& type = i->next();
         mEventController->unregisterObserver(this, type->getString());
      }
   }
   else
   {
      // find the event to remove and unregister
      bool removed = false;
      DynamicObjectIterator i = mEventTypes.getIterator();
      while(!removed && i->hasNext())
      {
         DynamicObject& type = i->next();
         if(strcmp(type->getString(), event) == 0)
         {
            // unregister the named event
            mEventController->unregisterObserver(this, event);
            i->remove();
            removed = true;
         }
      }
   }
}

void EventWaiter::eventOccurred(Event& e)
{
   mLock.lock();
   {
      // mark event occurred and notify all observers
      mEventOccurred = true;
      mEvents.push_back(e);
      mLock.notifyAll();
   }
   mLock.unlock();
}

bool EventWaiter::waitForEvent(uint32_t timeout)
{
   mLock.lock();
   {
      if(!mEventOccurred)
      {
         mLock.wait(timeout);
      }
   }
   mLock.unlock();

   return mEventOccurred;
}

Event EventWaiter::popEvent()
{
   Event e(NULL);

   mLock.lock();
   {
      if(!mEvents.empty())
      {
         e = mEvents.front();
         mEvents.pop_front();
      }

      if(mEvents.empty())
      {
         mEventOccurred = false;
      }
   }
   mLock.unlock();

   return e;
}
