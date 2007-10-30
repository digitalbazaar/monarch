/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_event_Observable_H
#define db_event_Observable_H

namespace db
{
namespace event
{

/**
 * An Observable is an object that produces events that can be observed by
 * an Observer. It makes use of a Modest OperationRunner to propogate its
 * events to all registered Observers.
 * 
 * @author Dave Longley
 */
class Observable
{
public:
   /**
    * Creates a new Observable.
    */
   Observable();
   
   /**
    * Destructs this Observable.
    */
   virtual ~Observable();
};

} // end namespace event
} // end namespace db
#endif
