/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_event_EventController_H
#define db_event_EventController_H

#include "db/event/Observable.h"
#include "db/util/DynamicObject.h"

namespace db
{
namespace event
{

/**
 * The Event Controller handles registering an observers and sending events.
 * 
 * @author Mike Johnson
 */
class EventController
{
protected:
   /**
    * The map of types to event id.
    */
   db::util::DynamicObject mTypeMap;
   
   /**
    * The Observable which will dispatch the events.
    */
   Observable mObservable;
   
   /**
    * The next event id to be assigned.
    */
   EventId mNextEventId;
   
   /**
    * Assigns an event id to an event type. If the event type already has an
    * event id, a new event id will not be set.
    * 
    * @param eventType the event type.
    */
   virtual void assignEventId(const char* eventType);
   
public:
   /**
    * Creates a new Event Controller.
    */
   EventController();
   
   /**
    * Destructs the Event Controller.
    */
   virtual ~EventController();
   
   /**
    * Registers an observer for certain event types. Any empty event types will
    * be ignored.
    *  
    * @param observer the new observer to register for events.
    * @param eventTypes the event types to register the observer for.
    */
   virtual void registerObserver(
      Observer* observer, db::util::DynamicObject& eventTypes);
   
   /**
    * Unregisters an observer for certain events, determined by the event types.
    * If an event type does not exist or is not set, it will be ignored.
    * 
    * @param observer the observer to unregister for events.
    * @param eventTypes the event types to unregister the observer for.
    */
   virtual void unregisterObserver(
      Observer* observer, db::util::DynamicObject& eventTypes);
   
   /**
    * Creates an association between two event types. Observers of the
    * parent event type will receieve any events sent to the child, but the
    * child observers will not receive any parent events.
    * 
    * @param child the event type to be sent to the parent.
    * @param parent the event type to receive events from the child event.
    */
   virtual void addParent(const char* child, const char* parent);
   
   /**
    * Removes the association between the child and parent event types.
    * Observers of the parent event will no longer receive the child events.
    * 
    * @param child the event type to stop sending to the parent.
    * @param parent the event type to stop receiving events from the child.
    */
   virtual void removeParent(const char* child, const char* parent);
   
   /**
    * Schedules an event. Each event should have its event type set so that it
    * can be delivered to registered observers of that event type. Events with
    * invalid or unregistered event types will be ignored. 
    * 
    * @param event the event to schedule for dispatching to observers.
    */
   virtual void schedule(Event& event);
};

} // end namespace event
} // end namespace db

#endif
