/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_event_Observer_H
#define db_event_Observer_H

#include "db/event/Event.h"

namespace db
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

} // end namespace event
} // end namespace db
#endif
