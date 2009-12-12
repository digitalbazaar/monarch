/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_Exception_H
#define monarch_rt_Exception_H

#include "monarch/rt/Collectable.h"
#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace rt
{

/**
 * An Exception is raised when some kind of unhandled behavior occurs.
 *
 * In the current implementation of monarch::rt, a single Exception is stored in
 * thread-local memory for each thread. Whenever an Object's method needs to
 * raise an Exception, it calls Exception::set() with a dynamically allocated
 * Exception (or derivative). The memory cleanup will be handled by the thread
 * when setting new exceptions and when the thread dies.
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
    * Returns true if this Exception is of the passed type.
    *
    * @param type the type to check.
    * @param startsWith false to only return true if the passed type string
    *        matches the exact type of this exception (the default behavior),
    *        true to return true if this exception's type starts with the
    *        given type.
    *
    * @return true if this Exception is of the passed type.
    */
   virtual bool isType(const char* type, bool startsWith = false);

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
    * Checks the cause stack/chain in this Exception for an Exception of
    * the given type. If one is found, it is returned. Otherwise, NULL
    * is returned.
    *
    * @param type the type to look for.
    * @param startsWith false to only return true if the passed type string
    *        matches the exact type of the cause (the default behavior),
    *        true to return true if the cause's type starts with the
    *        given type.
    *
    * @return the Exception with the given type, if found in this Exception's
    *         stack/chain of causes.
    */
   virtual Collectable<Exception> getCauseOfType(
      const char* type, bool startsWith = false);

   /**
    * Gets the details for this Exception. If there were no details
    * previously set, a DynamicObject will be initialized upon calling
    * this method.
    *
    * @return a reference to the details for this Exception.
    */
   virtual DynamicObject& getDetails();

   /**
    * Sets the exception for the current thread, replacing any old existing
    * exception.
    *
    * This will store the passed reference in thread-local memory, incrementing
    * its count. The thread-local reference will be cleared, decrementing the
    * count, when the current thread exits or when the exception reference is
    * replaced by another call to set() on the same thread.
    *
    * Note: If the exception to be set was caused by another related
    * exception, then a call to push() is more appropriate. Calling push()
    * will preserve an existing exception on the current thread as a cause
    * of the new exception that is passed to push.
    *
    * @param e the reference to the exception to set for the current thread.
    *
    * @return the reference to the exception.
    */
   static Collectable<Exception>& set(Collectable<Exception>& e);

   /**
    * Pushes an exception onto the current thread's stack of related exceptions.
    *
    * The previous existing exception (at the old top of the stack) will be
    * saved as the "cause" of the new exception. If there was no existing
    * exception, then there will be no cause set on the new exception.
    *
    * If the caller instead wishes to set an entirely new exception that is
    * unrelated to any previous exceptions set on the thread, then they should
    * call set() instead of push().
    *
    * @param e the reference to the exception to push onto the current thread,
    *          keeping any existing exception as the cause of "e".
    *
    * @return the reference to the exception.
    */
   static Collectable<Exception>& push(Collectable<Exception>& e);

   /**
    * Gets a reference to the exception for the current thread. This will be
    * the last exception that was set on this thread. It is stored in
    * thread-local memory the reference to it will be automatically cleared,
    * and thus decremented, when the thread exits.
    *
    * The returned exception may have a stack related exceptions, with
    * each layer stored as the "cause" of the exception above it.
    *
    * @return a reference to the last exception for the current thread, which
    *         may reference NULL.
    */
   static Collectable<Exception> get();

   /**
    * Returns true if the current thread has encountered an exception that
    * can be retrieved by calling Exception::get(), false if not.
    *
    * @return true if the current thread an exception, false if not.
    */
   static bool isSet();

   /**
    * Clears any exception from the current thread. This clears the
    * thread-local reference to the exception's memory (thus decrementing
    * its count).
    */
   static void clear();

   /**
    * Gets the last exception as a DynamicObject.
    *
    * @return the last exception as a DynamicObject.
    */
   static DynamicObject getAsDynamicObject();

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
} // end namespace monarch
#endif
