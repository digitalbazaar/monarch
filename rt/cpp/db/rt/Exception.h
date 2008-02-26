/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Exception_H
#define db_rt_Exception_H

#include "db/rt/Collectable.h"
#include <stddef.h>

namespace db
{
namespace rt
{

// forward declare DynamicObject
class DynamicObject;

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
   Collectable<Exception>* mCause;
   
   /**
    * Some key-value pairs with details about this exception.
    */
   DynamicObject* mDetails;
   
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
    * Sets the message for this Exception.
    *
    * @param message the message for this Exception.
    */
   virtual void setMessage(const char* message);
   
   /**
    * Gets the message for this Exception.
    *
    * @return the message for this Exception.
    */
   virtual const char* getMessage();
   
   /**
    * Sets the type for this Exception.
    *
    * @param type the type for this Exception.
    */
   virtual void setType(const char* type);
   
   /**
    * Gets the type for this Exception.
    *
    * @return the type for this Exception.
    */
   virtual const char* getType();
   
   /**
    * Sets the code for this Exception.
    *
    * @param code the code for this Exception.
    */
   virtual void setCode(int code);
   
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
    */
   virtual void setCause(Collectable<Exception>& cause);
   
   /**
    * Gets the cause for this Exception.
    * 
    * @return the cause for this Exception (may be NULL).
    */
   virtual Collectable<Exception>& getCause();
   
   /**
    * Gets the details for this Exception. If there were no details
    * previously set, a DynamicObject will be initialized upon calling
    * this method.
    * 
    * @return a reference to the details for this Exception.
    */
   virtual DynamicObject& getDetails();
   
   /**
    * Sets the exception for the current thread.
    * 
    * This will store the passed reference in thread-local memory, incrementing
    * its count. The thread-local reference will be cleared, decrementing the
    * count, when the current thread exits or when the exception reference is
    * replaced by another call to setException() on the same thread.
    * 
    * Note: An exception's cause may be set externally to this thread either
    * to a locally generated one or to the exception retrieved via getLast().
    * However, if the cause is the exception retrieved via getLast(), it is
    * faster to simply pass this method "true" for the "caused" parameter
    * than it is to manually get and set the cause.
    * 
    * @param e the reference to the Exception to set for the current thread.
    * @param caused true if the current Exception on the thread caused
    *               the passed Exception and should be set as its cause.
    * 
    * @return the reference to the Exception.
    */
   static Collectable<Exception>& setLast(
      Collectable<Exception>& e, bool caused);
   
   /**
    * Gets a reference to the Exception for the current thread. This will be
    * the last Exception that was set on this thread. It is stored in
    * thread-local memory the reference to it will be automatically cleared,
    * and thus decremented, when the thread exits.
    * 
    * @return a reference to the last Exception for the current thread, which
    *         may reference NULL.
    */
   static Collectable<Exception> getLast();
   
   /**
    * Returns true if the current thread has encountered an Exception that
    * can be retrieved by calling Thread::getException(), false if not.
    * 
    * @return true if the current thread an Exception, false if not.
    */
   static bool hasLast();
   
   /**
    * Clears any Exception from the current thread. This clears the
    * thread-local reference to the exception's memory (thus decrementing
    * its count).
    */
   static void clearLast();
   
   /**
    * Converts the passed Exception to a DynamicObject.
    * 
    * @param e the reference to the Exception to convert to a DynamicObject.
    * 
    * @return the passed Exception as a DynamicObject.
    */
   static DynamicObject convertToDynamicObject(Collectable<Exception>& e);
   
   /**
    * Converts the passed DynamicObject to an Exception. The returned
    * Exception will have its chain of causes set, if any.
    * 
    * @param dyno the DynamicObject to convert to an Exception.
    * 
    * @return the reference to the Exception.
    */
   static Collectable<Exception> convertToException(DynamicObject& dyno);
};

// define a reference counted Exception type
typedef Collectable<Exception> ExceptionRef;

} // end namespace rt
} // end namespace db
#endif
