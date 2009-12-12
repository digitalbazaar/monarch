/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_event_EventDaemon_H
#define db_event_EventDaemon_H

#include "monarch/event/EventController.h"

namespace db
{
namespace event
{

/**
 * An EventDaemon is a process that periodically schedules events with an
 * EventController. Events to be scheduled can be added as one-time events or
 * as repeating events.
 *
 * @author Dave Longley
 */
class EventDaemon : public monarch::rt::Runnable
{
protected:
   /**
    * EventData contains the information needed to schedule an event. This
    * includes an event to be scheduled, the scheduling interval in
    * milliseconds, the number of times to schedule the event, and how much
    * time remains until the event is scheduled again.
    */
   struct EventData
   {
      Event event;
      Event cloned;
      uint32_t interval;
      int count;
      uint32_t remaining;
      int refs;
      EventData(Event& e, uint32_t i, int c, int r);
   };

   /**
    * A list of event data.
    */
   typedef std::list<EventData> EventList;
   EventList mEvents;

   /**
    * A lock for modifying the EventList and waiting.
    */
   monarch::rt::ExclusiveLock mLock;

   /**
    * EventController to schedule events with.
    */
   EventController* mEventController;

   /**
    * The operation running this daemon.
    */
   monarch::modest::Operation mOperation;

   /**
    * Set to true when this daemon is running.
    */
   bool mRunning;

   /**
    * The time at which this daemon started waiting.
    */
   uint64_t mStartWaitTime;

   /**
    * True if the daemon should try to schedule events, false if not.
    */
   bool mScheduleEvents;

public:
   /**
    * Creates a new EventDaemon.
    */
   EventDaemon();

   /**
    * Destructs the EventDaemon.
    */
   virtual ~EventDaemon();

   /**
    * Reset this EventDaemon, dropping all of its added events.
    */
   virtual void reset();

   /**
    * Starts this EventDaemon. Events will be scheduled with the associated
    * EventController according to how they were added. More events may be
    * added or removed while the EventDaemon is running.
    *
    * @param opRunner the OperationRunner to use to run this daemon.
    * @param ec the EventController to use.
    */
   virtual void start(
      monarch::modest::OperationRunner* opRunner,
      EventController* ec);

   /**
    * Stops this EventDaemon. Events will no longer be scheduled but they will
    * not be removed from this daemon.
    */
   virtual void stop();

   /**
    * Adds an event to be scheduled at the current interval for the specified
    * number of times. If "refs" is specified and the same event at the same
    * interval already exists, then its reference count will be updated and
    * the passed count will be added to its remaining count or it will be
    * set to infinite if specified.
    *
    * @param e the event, with e["type"] set, to schedule.
    * @param interval the time at which to schedule the event, in milliseconds.
    * @param count the number of times to schedule the event, -1 for infinite.
    * @param refs 0 to add a duplicate event if the given event and interval
    *             interval have already been added, > 0 to add reference(s) to
    *             an existing event and interval.
    */
   virtual void add(Event& e, uint32_t interval, int count, int refs = 0);

   /**
    * Removes an event type from this daemon. Events of the passed type will
    * no longer be scheduled by this daemon.
    *
    * @param type the event type to remove.
    * @param refs 0 to remove all references with the given type, > 0 to remove
    *            references for the given type.
    */
   virtual void remove(const char* type, int refs = 0);

   /**
    * Removes a specific event from this daemon. It will no longer be scheduled
    * by this daemon.
    *
    * @param e the specific event to remove.
    * @param refs 0 to remove all references with the given event, > 0 to remove
    *             some references for the given event.
    */
   virtual void remove(Event& e, int refs = 0);

   /**
    * Removes a specific event from this daemon. It will no longer be scheduled
    * by this daemon.
    *
    * @param e the specific event to remove.
    * @param interval the event's previously set time interval.
    * @param refs 0 to remove all events references to the given event, > 0 to
    *             remove some references for the given event.
    */
   virtual void remove(Event& e, uint32_t interval, int refs = 0);

   /**
    * Runs this daemon.
    */
   virtual void run();
};

} // end namespace event
} // end namespace db

#endif
