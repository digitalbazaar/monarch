/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_event_ObserverDelegate_H
#define db_event_ObserverDelegate_H

#include "db/event/Observer.h"
#include "db/rt/Runnable.h"

namespace db
{
namespace event
{

/**
 * An ObserverDelegate is an Observer that delegates event handling to a
 * mapped function on some HandlerType. It can also be used as a Runnable
 * that can handle a single event.
 * 
 * @author Dave Longley
 */
template<typename HandlerType>
class ObserverDelegate : public Observer, public db::rt::Runnable
{
protected:
   /**
    * Typedef for handler's event function.
    */
   typedef void (HandlerType::*EventFunction)(Event& e);
   
   /**
    * Typedef for handler's event w/user-data function.
    */
   typedef void (HandlerType::*EventWithUserDataFunction)(
      Event& e, void* userData);
   
   /**
    * Typedef for freeing a handler's user-data.
    */
   typedef void (HandlerType::*FreeUserDataFunction)(void* userData);
   
   /**
    * The actual handler object.
    */
   HandlerType* mHandler;
   
   /**
    * The handler's event function.
    */
   EventFunction mFunction;
   
   /**
    * The handler's event w/user-data function.
    */
   EventWithUserDataFunction mUserDataFunction;
   
   /**
    * The handler's user-data.
    */
   void* mUserData;
   
   /**
    * The handler's free user-data function.
    */
   FreeUserDataFunction mFreeUserDataFunction;
   
   /**
    * An observer to handle an event with.
    */
   Observer* mObserver;
   
   /**
    * An event to handle.
    */
   Event mEvent;
   
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
    * Creates a new ObserverDelegate with the specified handler object and
    * function for handling an Event with some user-data.
    * 
    * @param h the actual handler object.
    * @param f the handler's function for handling an Event w/user-data.
    * @param userData the user-data to pass to the function when an
    *                 event occurs.
    * @param ff a function to call to free the passed user-data, NULL for none.
    */
   ObserverDelegate(
      HandlerType* h, EventWithUserDataFunction f, void* userData,
      FreeUserDataFunction ff = NULL);
   
   /**
    * Creates a new Runnable ObserverDelegate with the specified observer
    * and Event to handle.
    * 
    * @param observer the Observer to handle the Event with.
    * @param e the Event to handle.
    */
   ObserverDelegate(Observer* observer, Event& e);
   
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
   
   /**
    * Handles a single pre-set event.
    */
   virtual void run();
};

template<class HandlerType>
ObserverDelegate<HandlerType>::ObserverDelegate(
   HandlerType* h, EventFunction f) :
   mEvent(NULL)
{
   mHandler = h;
   mFunction = f;
   mUserDataFunction = NULL;
   mFreeUserDataFunction = NULL;
   mUserData = NULL;
   mObserver = NULL;
}

template<class HandlerType>
ObserverDelegate<HandlerType>::ObserverDelegate(
   HandlerType* h, EventWithUserDataFunction f,
   void* userData, FreeUserDataFunction ff) :
   mEvent(NULL)
{
   mHandler = h;
   mFunction = NULL;
   mUserDataFunction = f;
   mFreeUserDataFunction = ff;
   mUserData = userData;
   mObserver = NULL;
}

template<class HandlerType>
ObserverDelegate<HandlerType>::ObserverDelegate(
   Observer* observer, Event& e) :
   mEvent(e)
{
   mHandler = NULL;
   mFunction = NULL;
   mUserDataFunction = NULL;
   mFreeUserDataFunction = NULL;
   mUserData = NULL;
   mObserver = observer;
}

template<class HandlerType>
ObserverDelegate<HandlerType>::~ObserverDelegate()
{
   if(mFreeUserDataFunction != NULL && mUserData != NULL)
   {
      (mHandler->*mFreeUserDataFunction)(mUserData);
   }
}

template<class HandlerType>
void ObserverDelegate<HandlerType>::eventOccurred(Event& e)
{
   if(mFunction)
   {
      // call handle event function on handler
      (mHandler->*mFunction)(e);
   }
   else
   {
      // call handle event w/user-data function on handler
      (mHandler->*mUserDataFunction)(e, mUserData);
   }
}

template<class HandlerType>
void ObserverDelegate<HandlerType>::run()
{
   mObserver->eventOccurred(mEvent);
}

} // end namespace event
} // end namespace db
#endif
