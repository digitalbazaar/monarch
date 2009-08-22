/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_event_EventController_H
#define db_event_EventController_H

#include "db/event/Observable.h"
#include "db/rt/DynamicObject.h"
#include "db/rt/SharedLock.h"

namespace db
{
namespace event
{

/**
 * The EventController handles registering an observers and sending events.
 *
 * Event["type"] is used internally by EventController and is therefore
 * reserved on all events that use it.
 *
 * The event type "*" is also reserved and refers to all events.
 *
 * @author Mike Johnson
 * @author Dave Longley
 */
class EventController : protected Observable
{
protected:
   /**
    * The map of types to event id.
    */
   db::rt::DynamicObject mTypeMap;

   /**
    * The next event id to be assigned.
    */
   EventId mNextEventId;

   /**
    * A lock for manipulating the type map.
    */
   db::rt::SharedLock mMapLock;

   /**
    * Gets the event ID for the passed event type, assigning a new ID if
    * necessary.
    *
    * @param type the event type to get the event ID for.
    *
    * @return the event type's event ID.
    */
   virtual EventId getEventId(const char* type);

public:
   /**
    * Creates a new EventController.
    */
   EventController();

   /**
    * Destructs the EventController.
    */
   virtual ~EventController();

   /**
    * Registers an event type with this EventController. The passed event
    * type automatically be made a child of the top-level event type "*".
    *
    * @param type the event type to register.
    */
   virtual void registerEventType(const char* type);

   /**
    * Registers an observer for a certain event type. The passed event type
    * will be automatically registered with this EventController.
    *
    * A condition variable may be passed as a filter. It must be a map that
    * contains a subset of data that must be present in the event in order
    * for the observer to receive the event. The map's elements may themselves
    * contain other maps which will also be checked as subsets.
    *
    * @param observer the new observer to register for events.
    * @param type the event type to register the observer for.
    * @param filter an optional conditional subset Map that must be present
    *               in an event in order for it to be sent to the Observer.
    */
   virtual void registerObserver(
      Observer* observer, const char* type,
      db::rt::DynamicObject* filter = NULL);

   /**
    * Registers an observer for several events. The passed eventTypes object
    * must be an array that has entries that are strings that correspond to
    * event types, or entries that are maps with "type" set to the event
    * type and "filter" optionally set to an EventFilter.
    *
    * @param observer the new observer to register for events.
    * @param eventTypes the list of event types and filters to register the
    *                   observer for ([] of {"type": string,
    *                   "filter: EventFilter} OR string).
    */
   virtual void registerObserver(
      Observer* observer, db::rt::DynamicObject& eventTypes);

   /**
    * Unregisters an observer for a certain event type. If the event type
    * is not registered, it will be ignored.
    *
    * @param observer the observer to unregister for events.
    * @param type the event type to unregister the observer for.
    */
   virtual void unregisterObserver(Observer* observer, const char* type);

   /**
    * Unregisters an observer for a list of event types. If a given event type
    * is not registered, it will be ignored.
    *
    * @param observer the observer to unregister for events.
    * @param eventTypes the list of event types to unregister the observer for.
    */
   virtual void unregisterObserver(
      Observer* observer, db::rt::DynamicObject& eventTypes);

   /**
    * Unregisters an observer entirely.
    *
    * @param observer the observer to unregister for events.
    */
   using Observable::unregisterObserver;

   /**
    * Creates an association between two event types. Observers of the
    * parent event type will receive any events sent to the child, but the
    * child observers will not receive any parent events.
    *
    * Both the child and parent event types will be automatically registered.
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
    * can be delivered to registered observers of that event type.
    *
    * @param event the event to schedule for dispatching to observers.
    */
   virtual void schedule(Event& event);

   /**
    * Starts this Observable. This causes this Observable to start dispatching
    * events to its registered Observers.
    *
    * @param opRunner the OperationRunner to use to start this Observable.
    */
   using Observable::start;

   /**
    * Stops this Observable. This causes this Observable to stop dispatching
    * events to its registered Observers.
    */
   using Observable::stop;
};

} // end namespace event
} // end namespace db

#endif
