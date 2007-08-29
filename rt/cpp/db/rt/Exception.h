/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Exception_H
#define db_rt_Exception_H

#include <stddef.h>

namespace db
{
namespace rt
{

/**
 * An Exception is raised when some kind of unhandled behavior occurs.
 * 
 * In the current implementation of db::rt, a single Exception is stored in
 * thread-local memory for each thread. Whenever an Object's method needs to
 * raise an Exception, it calls Thread::setException() with a dynamically
 * allocated Exception (or derivative). The memory cleanup will be handled
 * by the thread when setting new exceptions and when the thread dies.
 * 
 * @author Dave Longley
 */
class Exception
{
protected:
   /**
    * A message for this Exception.
    */
   char* mMessage;

   /**
    * A code associated with this Exception.
    */
   char* mCode;
   
   /**
    * Sets the message for this Exception.
    *
    * @param message the message for this Exception.
    */
   virtual void setMessage(const char* message);
   
   /**
    * Sets the code for this Exception.
    *
    * @param code the code for this Exception.
    */
   virtual void setCode(const char* code);
   
public:
   /**
    * Creates a new Exception. A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   Exception(const char* message = NULL, const char* code = NULL);
   
   /**
    * Destructs this Exception.
    */
   virtual ~Exception();
   
   /**
    * Gets the message for this Exception.
    *
    * @return the message for this Exception.
    */
   virtual const char* getMessage();
   
   /**
    * Gets the code for this Exception.
    *
    * @return the code for this Exception.
    */
   virtual const char* getCode();
   
   /**
    * Sets the last Exception for the current thread. This will store the
    * passed exception in thread-local memory and delete it when the current
    * thread exits or when it is replaced by another call to setException()
    * on the same thread.
    * 
    * It is safe to call Exception::setLast(Exception::getLast()), no
    * memory will be mistakenly collected.
    * 
    * @param e the Exception to set for the current thread.
    */
   static void setLast(Exception* e);
   
   /**
    * Gets the last Exception for the current thread. This will be the last
    * Exception that was set on this thread. It is stored in thread-local
    * memory and automatically cleaned up when the thread exits.
    * 
    * @return the last Exception for the current thread, which may be NULL.
    */
   static Exception* getLast();
   
   /**
    * Returns true if the current thread has encountered an Exception that
    * can be retrieved by calling Exception::getLast(), false if not.
    * 
    * @return true if the current thread an Exception, false if not.
    */
   static bool hasLast();
   
   /**
    * Clears any Exception from the current thread.
    */
   static void clearLast();
};

} // end namespace rt
} // end namespace db
#endif
