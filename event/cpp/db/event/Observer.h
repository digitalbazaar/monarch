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
   Observer();
   
   /**
    * Destructs this Observer.
    */
   virtual ~Observer();
};

} // end namespace event
} // end namespace db
#endif
