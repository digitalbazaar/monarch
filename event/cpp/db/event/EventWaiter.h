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
 * An EventWaiter can wait for more than one event type by calling start()
 * multiple times with different event types. Once an event of any of the
 * event types occurs, the EventWaiter's waitForEvent() method will return.
 * 
 * The last event that occurred can be retrieved with getLastEvent().
 * 
 * @author David I. Lehn
 * @author Dave Longley
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
    * The types of events to wait on.
    */
   db::rt::DynamicObject mEventTypes;
   
   /**
    * A stack of events that have occurred.
    */
   typedef std::list<Event> EventStack;
   EventStack mEvents;
   
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
    * Reset the flag that an event occurred and clear the stored events.
    */
   virtual void reset();
   
   /**
    * Register this EventWaiter for the named events. As events occur, they
    * will be stored in a stack. More than one event may occur between the
    * time the EventWaiter starts and when it waits for an event.
    * 
    * @param event the event type to wait for.
    */
   virtual void start(const char* event);
   
   /**
    * Fire the first type of event. This is a convenience to fire the event
    * but it is not required to use this method.
    */
   virtual void fire();
   
   /**
    * Unregister this EventWaiter for the event registered with start().
    */
   virtual void stop();
   
   /**
    * Observer protocol that is called when the registered events occur.
    * Designed for internal use.
    */
   virtual void eventOccurred(Event& e);
   
   /**
    * Block waiting for the registered events to occur. If the waiting thread
    * is interrupted this may return false without the event occuring. In such
    * cases an exception may be set (see the ExclusiveLock documentation).
    * 
    * @return true if an event occurred, false if one did not.
    */
   virtual bool waitForEvent();
   
   /**
    * Returns the oldest event that occurred and removes it from the stack
    * of events received by this EventWaiter since it started.
    * 
    * @return the oldest event that occurred.
    */
   virtual Event popEvent();
};

} // end namespace event
} // end namespace db

#endif
