/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_event_EventDaemon_H
#define db_event_EventDaemon_H

#include "db/event/EventController.h"

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
class EventDaemon : public db::rt::Runnable
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
      uint32_t interval;
      int count;
      uint32_t remaining;
      EventData(Event& e, uint32_t i, int c);
   };
   
   /**
    * A list of event data.
    */
   typedef std::list<EventData> EventList;
   EventList mEvents;
   
   /**
    * A lock for modifying the EventList and waiting.
    */
   db::rt::ExclusiveLock mLock;
   
   /**
    * EventController to schedule events with.
    */
   EventController* mEventController;
   
   /**
    * The operation running this daemon.
    */
   db::modest::Operation mOperation;
   
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
      db::modest::OperationRunner* opRunner,
      EventController* ec);
   
   /**
    * Stops this EventDaemon. Events will no longer be scheduled but they will
    * not be removed from this daemon.
    */
   virtual void stop();
   
   /**
    * Adds an event to be scheduled at the current interval for the specified
    * number of times.
    * 
    * @param e the event, with e["type"] set, to schedule.
    * @param interval the time at which to schedule the event, in milliseconds.
    * @param count the number of times to schedule the event, -1 for infinite.
    */
   virtual void add(Event& e, uint32_t interval, int count);
   
   /**
    * Removes an event type from this daemon. Events of the passed type will
    * no longer be scheduled by this daemon.
    * 
    * @param type the event type to remove.
    */
   virtual void remove(const char* type);
   
   /**
    * Removes a specific event from this daemon. It will no longer be scheduled
    * by this daemon.
    * 
    * @param e the specific event to remove.
    */
   virtual void remove(Event& e);
   
   /**
    * Removes a specific event from this daemon. It will no longer be scheduled
    * by this daemon.
    * 
    * @param e the specific event to remove.
    * @param interval the event's previously set time interval.
    */
   virtual void remove(Event& e, uint32_t interval);
   
   /**
    * Runs this daemon.
    */
   virtual void run();
};

} // end namespace event
} // end namespace db

#endif
