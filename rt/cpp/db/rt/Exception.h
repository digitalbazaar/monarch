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
    * A type for this Exception.
    */
   char* mType;
   
   /**
    * A code associated with this Exception.
    */
   int mCode;
   
   /**
    * A cause associated with this Exception.
    */
   Exception* mCause;
   
   /**
    * True if the cause exception should be cleaned up on destruction, false
    * if not.
    */
   bool mCleanupCause;
   
   /**
    * Sets the message for this Exception.
    *
    * @param message the message for this Exception.
    */
   virtual void setMessage(const char* message);
   
   /**
    * Sets the type for this Exception.
    *
    * @param type the type for this Exception.
    */
   virtual void setType(const char* type);
   
   /**
    * Sets the code for this Exception.
    *
    * @param code the code for this Exception.
    */
   virtual void setCode(int code);
   
public:
   /**
    * Creates a new Exception. A message, type, and code may be optionally
    * specified.
    *
    * @param message the message for this Exception.
    * @param type the type for this Exception.
    * @param code the code for this Exception.
    */
   Exception(const char* message = "", const char* type = "", int code = 0);
   
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
    * Gets the type for this Exception.
    *
    * @return the type for this Exception.
    */
   virtual const char* getType();
   
   /**
    * Gets the code for this Exception.
    *
    * @return the code for this Exception.
    */
   virtual int getCode();
   
   /**
    * Sets the cause for this Exception.
    * 
    * @param cause the cause for this Exception.
    * @param cleanup true if this Exception should manage the memory for
    *                the passed cause, false if not.
    */
   virtual void setCause(Exception* cause, bool cleanup);
   
   /**
    * Gets the cause for this Exception.
    * 
    * @return the cause for this Exception (may be NULL).
    */
   virtual Exception* getCause();
   
   /**
    * Sets the last Exception for the current thread. This will store the
    * passed exception in thread-local memory and delete it when the current
    * thread exits or when it is replaced by another call to setException()
    * on the same thread, unless otherwise specified.
    * 
    * It is safe to call Exception::setLast(Exception::getLast()), no
    * memory will be mistakenly collected.
    * 
    * If the current exception is the cause of the passed exception, its
    * memory will not be mistakenly collected.
    * 
    * @param e the Exception to set for the current thread.
    * @param cleanup true to reclaim the memory for an existing exception,
    *                false to leave it alone.
    * 
    * @return the Exception set for the current thread (same as passed).
    */
   static Exception* setLast(Exception* e, bool cleanup = true);
   
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
    * Clears any Exception from the current thread, cleaning up the memory
    * if requested (this is done by default).
    * 
    * @param cleanup true if the Exception's memory should be reclaimed, false
    *                if not.
    */
   static void clearLast(bool cleanup = true);
};

} // end namespace rt
} // end namespace db
#endif
