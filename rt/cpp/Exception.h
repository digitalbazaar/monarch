/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Exception_H
#define Exception_H

#include <string>

#include "Object.h"

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
class Exception : public virtual Object
{
protected:
   /**
    * A message for this Exception.
    */
   std::string mMessage;

   /**
    * A code associated with this Exception.
    */
   std::string mCode;

public:
   /**
    * Creates a new Exception. A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   Exception(const std::string& message = "", const std::string& code = "");
   
   /**
    * Destructs this Exception.
    */
   virtual ~Exception();
   
   /**
    * Gets the message for this Exception.
    *
    * @return the message for this Exception.
    */
   const std::string& getMessage();

   /**
    * Gets the code for this Exception.
    *
    * @return the code for this Exception.
    */
   const std::string& getCode();
};

} // end namespace rt
} // end namespace db
#endif
