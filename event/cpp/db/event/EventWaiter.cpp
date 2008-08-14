/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/EventWaiter.h"

#include "db/rt/DynamicObjectIterator.h"

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
   stop();
}

void EventWaiter::reset()
{
   mEventOccurred = false;
   mEvents.clear();
}

void EventWaiter::start(const char* event)
{
   mEventTypes->append() = event;
   mEventController->registerObserver(this, event);
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
   lock();
   {
      // mark event occurred and notify all observers
      mEventOccurred = true;
      mEvents.push_back(e);
      notifyAll();
   }
   unlock();
}

bool EventWaiter::waitForEvent()
{
   lock();
   {
      if(!mEventOccurred)
      {
         wait();
      }
   }
   unlock();
   
   return mEventOccurred;
}

Event EventWaiter::popEvent()
{
   Event e(NULL);
   
   lock();
   {
      if(!mEvents.empty())
      {
         e = mEvents.front();
         mEvents.pop_front();
      }
   }
   unlock();
   
   return e;
}
