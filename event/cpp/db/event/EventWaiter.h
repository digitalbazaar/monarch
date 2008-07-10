/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_event_EventWaiter_H
#define db_event_EventWaiter_H

#include "db/event/Observable.h"
#include "db/event/EventController.h"

namespace db
{
namespace event
{

/**
 * An EventWaiter is used to provide a means for a thread to wait for an event
 * to be posted to an EventController.  As soon as the waiter's start() has
 * registered the event it can receive and store that the event was fired.  A
 * call to waitForEvent() will succeed with true immediately if the event fires
 * before the waitForEvent() is called.  If the event has not yet fired, then
 * waitForEvent() will block.  A call to reset() will reset the flag for the
 * event.
 * 
 * @author David I. Lehn
 */
class EventWaiter :
   virtual public db::rt::ExclusiveLock,
   public Observer
{
protected:
   /**
    * EventController to use to watch for event.
    */
   EventController* mEventController;

   /**
    * Name of the event to wait on.
    */
   char* mEvent;
   
   /**
    * Flag set when an event occurs.  Used to stop deadlock when event occurs
    * between start() and waitForEvent();
    */
   bool mEventOccurred;
   
   /**
    * Flag to keep track if this waiter is registered with an event controller.
    */
   bool mRegistered;

public:

   /**
    * Creates a new EventWaiter.
    * 
    * @param ec the EventController to use.
    */
   EventWaiter(EventController* ec);

   /**
    * Destructs the EventWaiter.
    */
   virtual ~EventWaiter();

   /**
    * Reset the flag that the event occurred.
    */
   virtual void reset();
   
   /**
    * Register this EventWaiter for the named event.
    * 
    * @param event the event type to wait for.
    */
   virtual void start(const char* event);
   
   /**
    * Fire the event.  This is a convienience to fire the event but it is not
    * required to use this method.
    */
   virtual void fire();
   
   /**
    * Unregister this EventWaiter for the event registered with start().
    */
   virtual void stop();
   
   /**
    * Observer protocol that is called when the registered event occurs.
    * Designed for internal use.
    */
   virtual void eventOccurred(Event& e);
   
   /**
    * Block waiting for the registered event to occur.  If the waiting thread
    * is interrupted this may return false without the event occuring.  In such
    * cases an exception may be set (see the Object documentation).
    * 
    * @return true if event occurred, false if it did not.
    */
   virtual bool waitForEvent();
};

} // end namespace event
} // end namespace db

#endif
