/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_IOEventDelegate_H
#define db_io_IOEventDelegate_H

#include "db/io/IOWatcher.h"

namespace db
{
namespace io
{

/**
 * An IOEventDelegate is a type of IOWatcher. It is a delegate class that wraps
 * a function that is called when an IOMonitor detects that a file descriptor
 * is ready to be read from or written to.
 *
 * @author Dave Longley
 */
template<typename T>
class IOEventDelegate : public IOWatcher
{
public:
   /**
    * Typedef for the callback function.
    */
   typedef void (T::*callback)(int fd, int flag);

   /**
    * The object with the callback function.
    */
   T* mObject;

   /**
    * The object's callback function.
    */
   callback mCallback;

public:
   /**
    * Creates a new IOEventDelegate that calls the passed callback function on
    * the passed object when a file descriptor is updated.
    *
    * @param obj the object with the callback function.
    * @param cb the callback function to call.
    */
   IOEventDelegate(T* obj, callback cb);

   /**
    * Destructs this IOEventDelegate.
    */
   virtual ~IOEventDelegate();

   /**
    * Called when a file descriptor is updated.
    *
    * @param fd the file descriptor that was updated.
    * @param events a bit flag describing what events (read/write) occurred.
    */
   virtual void fdUpdated(int fd, int events);

   /**
    * Sets the object and callback to use.
    *
    * @param obj the object with the callback function.
    * @param cb the callback function to call.
    */
   virtual void setCallback(T* obj, callback cb);
};

template<typename T>
IOEventDelegate<T>::IOEventDelegate(T* obj, callback cb)
{
   setCallback(obj, cb);
}

template<typename T>
IOEventDelegate<T>::~IOEventDelegate()
{
}

template<typename T>
void IOEventDelegate<T>::fdUpdated(int fd, int events)
{
   // call object's callback
   (mObject->*mCallback)(fd, events);
}

template<typename T>
void IOEventDelegate<T>::setCallback(T* obj, callback cb)
{
   mObject = obj;
   mCallback = cb;
}

} // end namespace io
} // end namespace db
#endif
