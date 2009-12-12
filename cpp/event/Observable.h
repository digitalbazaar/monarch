/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_event_Observable_H
#define db_event_Observable_H

#include "monarch/modest/OperationList.h"
#include "monarch/modest/OperationRunner.h"
#include "monarch/event/Observer.h"

#include <list>
#include <map>
#include <vector>

namespace db
{
namespace event
{

/**
 * An Observable is an object that produces events that can be observed by
 * an Observer. It makes use of a Modest OperationRunner to dispatch its
 * events to all registered Observers.
 *
 * Observers will receive events in the same order that they were generated,
 * unless the events are marked as "parallel", at which point they might
 * arrive at any time. The events are also dispatched "semi-simultaneously,"
 * meaning that multiple threads are used to dispatch the events to the
 * Observers.
 *
 * Events and Observers are associated with EventIds such that when an Event
 * is sent out, it will be sent out to all Observers that are registered with
 * the EventId it was sent out along with. EventIds can also be associated
 * with one another by creating "taps" such that, when an Event is scheduled
 * with one EventId, the Observers of another EventId will "tap" into its
 * distribution and receive it as well.
 *
 * For instance, two Observers could register for Events. The first could
 * register for Events with EventId 1 and the second could register for
 * Events with EventId 2. Then EventId 2 could be added as a "tap" for
 * EventId 1 such that when Events with EventId 1 are sent, they are also sent
 * to the Observers of EventId 2. Then the first Observer will only get
 * Events sent with EventId 1, but the second will receive events sent with
 * EventId 1 or EventId 2.
 *
 * Multiple taps may be added for any EventId. No checking is made to ensure
 * that Observers do not receive "double" events due to a poorly created
 * system of taps or due to registration under both a tap and its tapee.
 *
 * Note: It is a programmer error to create a situation where two Observers
 * are competing to unregister each other. It is also a programmer error,
 * when using parallel events, to create a situation where two events for
 * the same Observer attempt to unregister the Observer concurrently. Both
 * of these situations will cause deadlock and must be protected against
 * using appropriate locking mechanisms.
 *
 * @author Dave Longley
 */
class Observable : public db::rt::Runnable
{
protected:
   /**
    * A list of observers.
    */
   typedef std::vector<Observer*> ObserverList;

   /**
    * A map of an EventFilter to a list of Observers.
    */
   typedef std::map<EventFilter, ObserverList> FilterMap;

   /**
    * The queue of undispatched events.
    */
   typedef std::list<Event> EventQueue;
   EventQueue mEventQueue;

   /**
    * A multimap of EventIds to their taps. EventIds are always taps to
    * themselves.
    */
   typedef std::multimap<EventId, EventId> EventIdMap;
   EventIdMap mTaps;

   /**
    * The map of EventIds to FilterMaps with registered Observers. This map
    * contains all of the Observers for all of the different Event IDs.
    */
   typedef std::map<EventId, FilterMap> ObserverMap;
   ObserverMap mObservers;

   /**
    * The OperationRunner for running operations.
    */
   db::modest::OperationRunner* mOpRunner;

   /**
    * The Operation used to run this Observable.
    */
   db::modest::Operation mOperation;

   /**
    * The current list of Operations being used to process events.
    */
   db::modest::OperationList mOpList;

   /**
    * The dispatch condition. Set to true when events can be dispatched,
    * false when not.
    */
   bool mDispatch;

   /**
    * A counter for event sequence IDs.
    */
   uint64_t mSequenceId;

   /**
    * The queue lock is engaged while the event queue is being updated
    * or examined.
    */
   db::rt::ExclusiveLock mQueueLock;

   /**
    * The registration lock is engaged to while dispatching an event,
    * registration/unregistration of observers, tap modification, and
    * starting/stopping the dispatch operation. It is not engaged during
    * event processing to allow event handlers to register/unregister
    * observers.
    */
   db::rt::ExclusiveLock mRegistrationLock;

public:
   /**
    * Creates a new Observable.
    */
   Observable();

   /**
    * Destructs this Observable.
    */
   virtual ~Observable();

   /**
    * Registers an Observer with this Observable for the given EventId. The
    * Observer will immediately begin to receive to events from this
    * Observable that are sent out using the given EventId.
    *
    * A condition variable may be passed as a filter. It must be a map that
    * contains a subset of data that must be present in the event in order
    * for the observer to receive the event. The map's elements may themselves
    * contain other maps which will also be checked as subsets.
    *
    * @param observer the Observer to register.
    * @param id the EventId for the events to receive.
    * @param filter an optional conditional subset Map that must be present
    *               in an event in order for it to be sent to the Observer.
    */
   virtual void registerObserver(
      Observer* observer, EventId id, db::rt::DynamicObject* filter = NULL);

   /**
    * Unregisters an Observer from this Observable for the given EventId. The
    * Observer will no longer receive events from this Observable that are
    * sent out using the given EventId. There may be some residual events that
    * the Observer receives because they were enroute, but no new events
    * with the passed EventId will be dispatched to the Observer.
    *
    * @param observer the Observer to unregister.
    * @param id the EventId for the events to unregister from.
    */
   virtual void unregisterObserver(Observer* observer, EventId id);

   /**
    * Unregisters an Observer from this Observable for all EventIds.
    *
    * @param observer the Observer to unregister.
    */
   virtual void unregisterObserver(Observer* observer);

   /**
    * Adds an EventId tap for the given EventId. This means that when events
    * are dispatched to Observers registered with the passed "id" they will
    * also be dispatched to Observers with the passed "tap".
    *
    * Multiple EventIds can be "tapped" by the same tap. For example, EventId
    * 1 and EventId 2 can have EventId 3 added as a tap to each of them using
    * addTap(1, 3) and addTap(2, 3). Given this scenario, if an event is
    * scheduled with EventId 1, it will be sent to the Observers of EventIds
    * 1 and 3. If an event is scheduled with EventId 2, it will be sent to
    * the Observers of EventIds 2 and 3. If an event is scheduled with EventId
    * 3, it will only be scheduled with the Observers of EventId 3.
    *
    * @param id the EventId to add a tap to.
    * @param tap the EventId tap to add to the passed EventId.
    */
   virtual void addTap(EventId id, EventId tap);

   /**
    * Removes an EventId tap for the given EventId.
    *
    * @param id the EventId to remove a tap from.
    * @param tap the EventId tap to remove from the passed EventId.
    */
   virtual void removeTap(EventId id, EventId tap);

   /**
    * Schedules an Event for dispatch. The Event can be dispatched
    * asynchronously, or synchronously. If it is sent synchronously then
    * it will be dispatched immediately, blocking the current thread until
    * it is has been processed by all associated Observers.
    *
    * By default, events will be scheduled for asynchronous dispatch.
    *
    * @param e the Event to schedule.
    * @param id the EventId for the Event.
    * @param async true for asynchronous dispatch, false for synchronous.
    */
   virtual void schedule(Event e, EventId id, bool async = true);

   /**
    * Starts this Observable. This causes this Observable to start dispatching
    * events to its registered Observers.
    *
    * @param opRunner the OperationRunner to use to start this Observable.
    */
   virtual void start(db::modest::OperationRunner* opRunner);

   /**
    * Stops this Observable. This causes this Observable to stop dispatching
    * events to its registered Observers.
    */
   virtual void stop();

   /**
    * Runs this Observable by dispatching its events to its registered
    * Observers.
    */
   virtual void run();

protected:
   /**
    * A helper function to removes an observer from a FilterMap.
    *
    * This method assumes the registration lock is engaged.
    *
    * @param observer the observer to remove.
    * @param fm the FilterMap to remove the observer from.
    * @param removableFilters a list of filters that can be removed now
    *                         that they contain no observers.
    */
   virtual void removeObserverFromFilterMap(
      Observer* observer, FilterMap& fm,
      std::vector<EventFilter>& removableFilters);

   /**
    * A recursive helper function for dispatching a single event to all
    * associated Observers.
    *
    * @param e the Event to dispatch.
    * @param id the EventId to dispatch it under.
    * @param waitList the OperationList to store event-handling Operations that
    *                 must be waited on to complete.
    */
   virtual void dispatchEvent(
      Event& e, EventId id, db::modest::OperationList& waitList);

   /**
    * Dispatches a single event to all associated Observers and waits
    * for them to finish processing the event.
    *
    * @param e the Event to dispatch.
    */
   virtual void dispatchEvent(Event& e);

   /**
    * Dispatches the events in the event queue to all registered Observers.
    */
   virtual void dispatchEvents();
};

} // end namespace event
} // end namespace db
#endif
