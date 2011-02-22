/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/Exception.h"

#include "monarch/rt/Thread.h"
#include "monarch/rt/DynamicObject.h"

#include <cstdlib>

using namespace monarch::rt;

Exception::Exception(const char* message, const char* type)
{
   mMessage = (message == NULL) ? NULL : strdup(message);
   mType = (type == NULL) ? NULL : strdup(type);
   mCause = new ExceptionRef(NULL);
   mDetails = new DynamicObject(NULL);
}

Exception::~Exception()
{
   free(mMessage);
   free(mType);
   delete mCause;
   delete mDetails;
}

void Exception::setMessage(const char* message)
{
   free(mMessage);
   mMessage = (message == NULL) ? NULL : strdup(message);
}

const char* Exception::getMessage()
{
   return (mMessage == NULL) ? "" : mMessage;
}

void Exception::setType(const char* type)
{
   free(mType);
   mType = (type == NULL) ? NULL : strdup(type);
}

const char* Exception::getType()
{
   return (mType == NULL) ? "" : mType;
}

bool Exception::isType(const char* type, bool startsWith, int n)
{
   bool rval;

   // just check prefix
   if(startsWith)
   {
      // calculate string length if not specified
      if(n == -1)
      {
         n = strlen(type);
      }
      rval = (strncmp(getType(), type, n) == 0);
   }
   else
   {
      rval = (strcmp(getType(), type) == 0);
   }

   return rval;
}

bool Exception::hasType(const char* type, bool startsWith, int n)
{
   bool rval;

   // optimization to pre-calculate and reuse the type length
   if(startsWith && n == -1)
   {
      n = strlen(type);
   }

   // check this exception's cause
   rval = isType(type, startsWith, n);

   // check deeper in the stack/chain
   if(!rval)
   {
      rval = hasCauseOfType(type, startsWith, n);
   }

   return rval;
}

void Exception::setCause(ExceptionRef& cause)
{
   *mCause = cause;
}

ExceptionRef& Exception::getCause()
{
   return *mCause;
}

bool Exception::hasCauseOfType(const char* type, bool startsWith, int n)
{
   bool rval = false;

   ExceptionRef& cause = getCause();
   if(!cause.isNull())
   {
      rval = cause->hasType(type, startsWith, n);
   }

   return rval;
}

DynamicObject& Exception::getDetails()
{
   if(mDetails->isNull())
   {
      DynamicObject details;
      details->setType(Map);
      *mDetails = details;
   }

   return *mDetails;
}

ExceptionRef& Exception::set(ExceptionRef& e)
{
   // false = do not use previous exception as cause, instead, clear it
   Thread::setException(e, false);
   return e;
}

ExceptionRef& Exception::push(ExceptionRef& e)
{
   // true = use previous exception as cause, do NOT clear it
   Thread::setException(e, true);
   return e;
}

ExceptionRef Exception::get()
{
   return Thread::getException();
}

bool Exception::isSet()
{
   return Thread::hasException();
}

void Exception::clear()
{
   Thread::clearException();
}

DynamicObject Exception::getAsDynamicObject()
{
   ExceptionRef e = Exception::get();
   return Exception::convertToDynamicObject(e);
}

DynamicObject Exception::convertToDynamicObject(ExceptionRef& e)
{
   DynamicObject dyno;

   dyno["message"] = e->getMessage();
   dyno["type"] = e->getType();

   if(!e->getCause().isNull())
   {
      dyno["cause"] = convertToDynamicObject(e->getCause());
   }

   if(!(*e).mDetails->isNull())
   {
      dyno["details"] = e->getDetails();
   }

   return dyno;
}

ExceptionRef Exception::convertToException(DynamicObject& dyno)
{
   ExceptionRef rval = new Exception(
      dyno["message"]->getString(),
      dyno["type"]->getString());

   if(dyno->hasMember("cause"))
   {
      ExceptionRef cause = convertToException(dyno["cause"]);
      rval->setCause(cause);
   }

   if(dyno->hasMember("details"))
   {
      *(*rval).mDetails = dyno["details"].clone();
   }

   return rval;
}
