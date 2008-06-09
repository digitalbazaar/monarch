/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Object_H
#define db_rt_Object_H

#include <string>

#include "db/rt/Monitor.h"

namespace db
{
namespace rt
{

/**
 * An Object represents a single synchronizable object in Digital Bazaar's
 * multi-threaded object-oriented runtime system.
 *
 * @author Dave Longley
 */
class Object
{
protected:
   /**
    * The Monitor for this Object.
    */
   Monitor mMonitor;
   
public:
   /**
    * Constructs a new Object.
    */
   Object();
   
   /**
    * Destructs this Object.
    */
   virtual ~Object();
   
   /**
    * Returns true if this Object equals the passed Object.
    *
    * @return true if this Object equals the passed Object.
    */
   virtual bool equals(const Object& obj) const;
   
   /**
    * Obtains the lock on this Object for the currently executing Thread.
    */
   virtual void lock();
   
   /**
    * Releases the lock on this Object.
    */
   virtual void unlock();
   
   /**
    * Notifies one Thread that is waiting on this Object's lock to wake up.
    *
    * This Object must be locked by calling lock() before executing this
    * method. It must be unlocked after executing it.
    */
   virtual void notify();
   
   /**
    * Notifies all Threads that are waiting on this Object's lock to wake up.
    *
    * This Object must be locked by calling lock() before executing this
    * method. It must be unlocked after executing it.
    */
   virtual void notifyAll();
   
   /**
    * Causes the current Thread to wait until another Thread calls
    * notify() or notifyAll() on this Object or until the passed number
    * of milliseconds pass.
    *
    * This Object must be locked by calling lock() before executing this
    * method. It must be unlocked after executing it.
    *
    * @param timeout the number of milliseconds to wait for a notify call
    *                before timing out, 0 to wait indefinitely.
    * 
    * @return false if the current thread is interrupted while waiting (with
    *         an InterruptedException set), true if not.
    */
   virtual bool wait(unsigned long timeout = 0);
   
   /**
    * Compares this Object to another Object. If the other Object is equal
    * to this Object, this method returns true.
    *
    * The equality of Objects is determined by their locations in memory. One
    * Object is equal to another if they use the same physical memory.
    *
    * @param rhs the Object to compare with this one.
    *
    * @return true if the passed Object equals this one.
    */
   virtual bool operator==(const Object &rhs) const;
};

} // end namespace rt
} // end namespace db

#endif
