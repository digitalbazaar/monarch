/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_event_ObserverList_H
#define db_event_ObserverList_H

#include "monarch/event/EventController.h"

namespace db
{
namespace event
{

/**
 * An ObserverList is a utility for keeping track of multiple Observers in a
 * single location and allows a list of Observers to be bulk unregistered from
 * a particular Observable. This list is particularly useful when used in
 * conjunction with ObserverRefs that point at ObserverDelegates. The use
 * of the list in this fashion allows a single class to delegate specific
 * events to specific methods in the class all whilst keeping the memory
 * management (allocation and cleanup of ObserverDelegates) simple.
 *
 * An example of the typical use of this list:
 *
 * class MyClass
 * {
 * protected:
 *    ObserverList mObserverList;
 *
 * public:
 *    void handleEventTypeA()
 *    {
 *       // handle event type A
 *    }
 *
 *    void handleEventTypeB()
 *    {
 *       // handle event type B
 *    }
 *
 *    void handleEventTypeC()
 *    {
 *       // handle event type C
 *    }
 *
 *    void myInitializeFn(Observable* observable)
 *    {
 *       ObserverRef h1(new ObserverDelegate<MyClass>(
 *          this, &MyClass::handleEventTypeA));
 *
 *       ObserverRef h2(new ObserverDelegate<MyClass>(
 *          this, &MyClass::handleEventTypeB));
 *
 *       ObserverRef h3(new ObserverDelegate<MyClass>(
 *          this, &MyClass::handleEventTypeC));
 *
 *       observable->registerObserver(&(*h1), 1);
 *       observable->registerObserver(&(*h2), 2);
 *       observable->registerObserver(&(*h3), 3);
 *
 *       mObserverList.add(h1);
 *       mObserverList.add(h2);
 *       mObserverList.add(h3);
 *    }
 *
 *    void myCleanupFn(Observable* observable)
 *    {
 *       mObserverList.unregisterFrom(observable);
 *       mObserverList.clear();
 *    }
 * };
 *
 * @author Dave Longley
 */
class ObserverList
{
protected:
   /**
    * The underlying list of Observers.
    */
   std::list<Observer*> mObservers;

   /**
    * A list of ObserverRefs.
    */
   std::list<ObserverRef> mObserverRefs;

public:
   /**
    * Creates a new ObserverList.
    */
   ObserverList();

   /**
    * Destructs this ObserverList.
    */
   virtual ~ObserverList();

   /**
    * Adds an Observer to this list.
    *
    * @param observer the Observer to add.
    */
   virtual void add(Observer* observer);

   /**
    * Adds a reference-counted Observer to this list.
    *
    * @param observer the Observer to add.
    */
   virtual void add(ObserverRef& observer);

   /**
    * Unregisters all Observers in this list from the passed Observable.
    *
    * @param observable the Observable to unregister from.
    */
   virtual void unregisterFrom(Observable* observable);

   /**
    * Unregisters all Observers in this list from the passed Observable for
    * the passed EventId.
    *
    * @param observable the Observable to unregister from.
    * @param eventId the event ID to unregister from.
    */
   virtual void unregisterFrom(Observable* observable, EventId id);

   /**
    * Unregisters all Observers in this list from the passed EventController.
    *
    * @param ec the EventController to unregister from.
    */
   virtual void unregisterFrom(EventController* ec);

   /**
    * Unregisters all Observers in this list from the passed EventController
    * for a certain event type.
    *
    * @param type the event type to unregister all observers for.
    */
   virtual void unregisterFrom(EventController* ec, const char* type);

   /**
    * Unregisters all Observers in this list from the passed EventController
    * for a list of event types.
    *
    * @param eventTypes the list of event types to unregister all observers for.
    */
   virtual void unregisterFrom(
      EventController* ec, monarch::rt::DynamicObject& eventTypes);

   /**
    * Clears this list of all of its Observers.
    *
    * Warning: This will not unregister the Observers from any Observables they
    * are registered with which may result in memory access violations for
    * Observers that were added to this list as ObserverRefs. Make sure to
    * unregister the Observers in this list from all Observables before
    * clearing it unless you know what you're doing.
    */
   virtual void clear();
};

} // end namespace event
} // end namespace db
#endif
