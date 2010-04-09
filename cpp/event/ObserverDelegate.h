/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_event_ObserverDelegate_H
#define monarch_event_ObserverDelegate_H

#include "monarch/event/Observer.h"
#include "monarch/rt/Runnable.h"

namespace monarch
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
class ObserverDelegate : public Observer, public monarch::rt::Runnable
{
protected:
   /**
    * Enum for types of event delegate.
    */
   enum Type
   {
      EventOnly,
      EventWithParam,
      EventWithDyno,
      EventRunnable
   };

   /**
    * Typedef for handler's event function.
    */
   typedef void (HandlerType::*EventFunction)(Event&);

   /**
    * Typedef for handler's event w/user-data function.
    */
   typedef void (HandlerType::*EventWithParamFunction)(Event&, void*);

   /**
    * Typedef for handler's event w/dyno function.
    */
   typedef void (HandlerType::*EventWithDynoFunction)(
      Event&, monarch::rt::DynamicObject&);

   /**
    * Typedef for freeing a handler's user-data.
    */
   typedef void (HandlerType::*FreeParamFunction)(void* param);

   /**
    * The type of event delegate.
    */
   Type mType;

   /**
    * The actual handler object.
    */
   HandlerType* mHandler;

   /**
    * Data for an event only function.
    */
   struct EventOnlyData
   {
      EventFunction handleFunction;
   };

   /**
    * Data for an event w/user-data function.
    */
   struct EventWithParamData
   {
      EventWithParamFunction handleFunction;
      FreeParamFunction freeFunction;
      void* param;
   };

   /**
    * Data for an event w/dyno function.
    */
   struct EventWithDynoData
   {
      EventWithDynoFunction handleFunction;
      monarch::rt::DynamicObject* param;
   };

   /**
    * Data for a runnable event delegate.
    */
   struct EventRunnableData
   {
      Observer* observer;
      Event* event;
   };

   /**
    * The type-specific data.
    */
   union
   {
      EventOnlyData* mEventOnly;
      EventWithParamData* mEventWithParam;
      EventWithDynoData* mEventWithDyno;
      EventRunnableData* mEventRunnable;
   };

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
    * @param param the user-data to pass to the function when an event occurs.
    * @param ff a function to call to free the passed user-data, NULL for none.
    */
   ObserverDelegate(
      HandlerType* h, EventWithParamFunction f, void* param,
      FreeParamFunction ff = NULL);

   /**
    * Creates a new ObserverDelegate with the specified handler object and
    * function for handling an Event with a DynamicObject.
    *
    * @param h the actual handler object.
    * @param f the handler's function for handling an Event w/user-data.
    * @param param the DynamicObject to pass to the function when the
    *           event occurs.
    */
   ObserverDelegate(
      HandlerType* h,
      EventWithDynoFunction f, monarch::rt::DynamicObject& param);

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
   mType(EventOnly),
   mHandler(h)
{
   mEventOnly = new EventOnlyData;
   mEventOnly->handleFunction = f;
}

template<class HandlerType>
ObserverDelegate<HandlerType>::ObserverDelegate(
   HandlerType* h, EventWithParamFunction f,
   void* param, FreeParamFunction ff) :
   mType(EventWithParam),
   mHandler(h)
{
   mEventWithParam = new EventWithParamData;
   mEventWithParam->handleFunction = f;
   mEventWithParam->freeFunction = ff;
   mEventWithParam->param = param;
}

template<class HandlerType>
ObserverDelegate<HandlerType>::ObserverDelegate(
   HandlerType* h, EventWithDynoFunction f, monarch::rt::DynamicObject& param) :
   mType(EventWithDyno),
   mHandler(h)
{
   mEventWithDyno = new EventWithDynoData;
   mEventWithDyno->handleFunction = f;
   mEventWithDyno->param = new monarch::rt::DynamicObject(param);
}

template<class HandlerType>
ObserverDelegate<HandlerType>::ObserverDelegate(Observer* observer, Event& e) :
   mType(EventRunnable)
{
   mEventRunnable = new EventRunnableData;
   mEventRunnable->observer = observer;
   mEventRunnable->event = new Event(e);
}

template<class HandlerType>
ObserverDelegate<HandlerType>::~ObserverDelegate()
{
   switch(mType)
   {
      case EventOnly:
         delete mEventOnly;
         break;
      case EventWithParam:
         if(mEventWithParam->param != NULL &&
            mEventWithParam->freeFunction != NULL)
         {
            (mHandler->*(mEventWithParam->freeFunction))(
               mEventWithParam->param);
         }
         delete mEventWithParam;
         break;
      case EventWithDyno:
         delete mEventWithDyno->param;
         delete mEventWithDyno;
         break;
      case EventRunnable:
         delete mEventRunnable->event;
         delete mEventRunnable;
         break;
   }
}

template<class HandlerType>
void ObserverDelegate<HandlerType>::eventOccurred(Event& e)
{
   switch(mType)
   {
      case EventOnly:
         (mHandler->*(mEventOnly->handleFunction))(e);
         break;
      case EventWithParam:
         (mHandler->*(mEventWithParam->handleFunction))(
            e, mEventWithParam->param);
         break;
      case EventWithDyno:
         (mHandler->*(mEventWithDyno->handleFunction))(
            e, *mEventWithDyno->param);
         break;
      case EventRunnable:
         // nothing to do here, event is fired from run()
         break;
   }
}

template<class HandlerType>
void ObserverDelegate<HandlerType>::run()
{
   mEventRunnable->observer->eventOccurred(*mEventRunnable->event);
}

} // end namespace event
} // end namespace monarch
#endif
