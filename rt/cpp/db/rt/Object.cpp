/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/Object.h"

#include "db/rt/Thread.h"

using namespace db::rt;

Object::Object()
{
}

Object::~Object()
{
}

bool Object::equals(const Object& obj) const
{
   bool rval = false;
   
   // compare memory locations
   if(this == &obj)
   {
      rval = true;
   }
   
   return rval;
}

inline void Object::lock()
{
   mMonitor.enter();
}

inline void Object::unlock()
{
   mMonitor.exit();
}

inline void Object::notify()
{
   mMonitor.notify();
}

inline void Object::notifyAll()
{
   mMonitor.notify();
}

inline bool Object::wait(unsigned long timeout)
{
   // instruct the current thread to wait to enter this Object's monitor
   return Thread::waitToEnter(&mMonitor, timeout);
}

inline bool Object::operator==(const Object &rhs) const
{
   return equals(rhs);
}
