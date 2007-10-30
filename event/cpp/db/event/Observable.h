/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_event_Observable_H
#define db_event_Observable_H

#include "db/rt/Object.h"
#include "db/event/Observer.h"

#include <list>

namespace db
{
namespace event
{

/**
 * An Observable is an object that produces events that can be observed by
 * an Observer. It makes use of a Modest OperationRunner to dispatch its
 * events to all registered Observers.
 * 
 * Observers will receive events in the same order that they were generated.
 * The events are also dispatched "semi-simultaneously," meaning that multiple
 * threads are used to dispatch the events to the Observers in the order that
 * the Observers were registered with the Observable.
 * 
 * @author Dave Longley
 */
class Observable : public virtual db::rt::Object
{
protected:
   /**
    * The queue of undispatched events.
    */
   typedef std::list<Event*> EventQueue;
   EventQueue mEventQueue;
   
   /**
    * The list of registered Observers.
    */
   typedef std::list<Observer*> ObserverList;
   ObserverList mObservers;
   
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
    * Registers an Observer with this Observable. The Observer will immediately
    * begin to receive to events from this Observable.
    * 
    * @param observer the Observer to register.
    */
   virtual void registerObserver(Observer* observer);
   
   /**
    * Unregisters an Observer from this Observable. The Observer will no
    * longer receive events from this Observable. There may be some residual
    * events that the Observer receives because they were enroute, but no
    * new events will be dispatched to the Observer.
    * 
    * @param observer the Observer to unregister.
    */
   virtual void unregisterObserver(Observer* observer);
   
   // FIXME: run observer on 
};

} // end namespace event
} // end namespace db
#endif
