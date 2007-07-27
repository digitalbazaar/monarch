/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Object.h"
#include "Thread.h"

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

void Object::lock()
{
   mMonitor.enter();
}

void Object::unlock()
{
   mMonitor.exit();
}

void Object::notify()
{
   mMonitor.notify();
}

void Object::notifyAll()
{
   mMonitor.notify();
}

InterruptedException* Object::wait(unsigned long timeout)
{
   // instruct the current thread to wait to enter this Object's monitor
   return Thread::waitToEnter(&mMonitor, timeout);
}

bool Object::operator==(const Object &rhs) const
{
   return equals(rhs);
}
