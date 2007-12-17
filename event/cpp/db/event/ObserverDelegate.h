/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_event_ObserverDelegate_H
#define db_event_ObserverDelegate_H

#include "db/event/Observer.h"

namespace db
{
namespace event
{

/**
 * An ObserverDelegate is an Observer that delegates event handling to a
 * mapped function on some HandlerType.
 * 
 * @author Dave Longley
 */
template<typename HandlerType>
class ObserverDelegate : public Observer
{
protected:
   /**
    * Typedef for handler's event function.
    */
   typedef void (HandlerType::*EventFunction)(Event& e);
   
   /**
    * The actual handler object.
    */
   HandlerType* mHandler;
   
   /**
    * The handler's event function.
    */
   EventFunction mFunction;
   
public:
   /**
    * Creates a new ObserverDelegate with the specified handler object and
    * function for handling an Event.
    * 
    * @param h the actual handler object.
    * @param f the handler's function for handling an Event.
    */
   ObserverDelegate(HandlerType* h, EventFunction f);
   
   /**
    * Destructs this ObserverDelegate.
    */
   virtual ~ObserverDelegate();
   
   /**
    * Handles the passed Event.
    * 
    * @param e the Event to handle.
    */
   virtual void eventOccurred(Event& e);
};

template<class HandlerType>
ObserverDelegate<HandlerType>::ObserverDelegate(HandlerType* h, EventFunction f)
{
   mHandler = h;
   mFunction = f;
}

template<class HandlerType>
ObserverDelegate<HandlerType>::~ObserverDelegate()
{
}

template<class HandlerType>
void ObserverDelegate<HandlerType>::eventOccurred(Event& e)
{
   // call handle event function on handler
   (mHandler->*mFunction)(e);
}

} // end namespace event
} // end namespace db
#endif
