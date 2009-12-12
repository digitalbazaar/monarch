/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_event_Observer_H
#define monarch_event_Observer_H

#include "monarch/event/Event.h"

namespace monarch
{
namespace event
{

/**
 * An Observer can register with an Observable to receive the events it
 * generates.
 *
 * @author Dave Longley
 */
class Observer
{
public:
   /**
    * Creates a new Observer.
    */
   Observer() {};

   /**
    * Destructs this Observer.
    */
   virtual ~Observer() {};

   /**
    * Called when an Event occurs on an Observable that this Observer is
    * registered with.
    *
    * @param e the Event that occurred.
    */
   virtual void eventOccurred(Event& e) = 0;
};

// type definition for a reference-counted Observer
typedef monarch::rt::Collectable<Observer> ObserverRef;

} // end namespace event
} // end namespace monarch
#endif
