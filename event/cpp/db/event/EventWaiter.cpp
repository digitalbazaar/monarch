/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/event/EventWaiter.h"

using namespace std;
using namespace db::event;
using namespace db::rt;

EventWaiter::EventWaiter(EventController* ec)
{
   mEventController = ec;
   mEvent = NULL;
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
   mEvent = strdup(event);
   mEventController->registerObserver(this, mEvent);
   mRegistered = true;
}

void EventWaiter::fire()
{
   // fire off this event
   Event e;
   e["type"] = mEvent;
   mEventController->schedule(e);
}

void EventWaiter::stop()
{
   if(mRegistered)
   {
      mEventController->unregisterObserver(this, mEvent);
      mRegistered = false;
      if(mEvent != NULL)
      {
         free(mEvent);
         mEvent = NULL;
      }
      reset();
   }
}

void EventWaiter::eventOccurred(Event& e)
{
   lock();
   {
      // mark event occurred and notify all observers
      mEventOccurred = true;
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
