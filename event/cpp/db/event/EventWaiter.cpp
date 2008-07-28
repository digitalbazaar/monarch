/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/EventWaiter.h"

#include "db/rt/DynamicObjectIterator.h"

#include <cstdlib>

using namespace std;
using namespace db::event;
using namespace db::rt;

EventWaiter::EventWaiter(EventController* ec) :
   mEventTypes(NULL)
{
   mEventController = ec;
   mRegistered = false;
   reset();
}

EventWaiter::~EventWaiter()
{
   stop();
}

void EventWaiter::reset()
{
   mEventOccurred = false;
}

void EventWaiter::start(const char* event)
{
   if(mEventTypes.isNull())
   {
      mEventTypes = DynamicObject();
      mEventTypes->append() = event;
   }
   
   mEventController->registerObserver(this, event);
   mRegistered = true;
}

void EventWaiter::fire()
{
   // fire off the first event
   Event e;
   e["type"] = mEventTypes[0]->getString();
   mEventController->schedule(e);
}

void EventWaiter::stop()
{
   if(mRegistered)
   {
      // unregister all events
      DynamicObjectIterator i = mEventTypes.getIterator();
      while(i->hasNext())
      {
         DynamicObject& type = i->next();
         mEventController->unregisterObserver(this, type->getString());
      }
      
      mRegistered = false;
      mEventTypes.setNull();
      reset();
   }
}

void EventWaiter::eventOccurred(Event& e)
{
   lock();
   {
      // mark event occurred and notify all observers
      mEventOccurred = true;
      mEvent = e;
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

Event EventWaiter::getLastEvent()
{
   Event e(NULL);
   
   lock();
   {
      e = mEvent;
   }
   unlock();
   
   return e;
}
