/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/rt/Exception.h"

#include "db/rt/Thread.h"
#include "db/rt/DynamicObject.h"

#include <cstdlib>

using namespace db::rt;

Exception::Exception(const char* message, const char* type, int code)
{
   mMessage = strdup((message == NULL) ? "" : message);
   mType = strdup((type == NULL) ? "" : type);
   mCode = code;
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
   mMessage = strdup((message == NULL) ? "" : message);
}

const char* Exception::getMessage()
{
   return mMessage;
}

void Exception::setType(const char* type)
{
   free(mType);
   mType = strdup((type == NULL) ? "" : type);
}

const char* Exception::getType()
{
   return mType;
}

void Exception::setCode(int code)
{
   mCode = code;
}
int Exception::getCode()
{
   return mCode;
}

void Exception::setCause(ExceptionRef& cause)
{
   *mCause = cause;
}

ExceptionRef& Exception::getCause()
{
   return *mCause;
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

// FIXME: deprecated
ExceptionRef& Exception::setLast(ExceptionRef& e, bool caused)
{
   caused ? Exception::push(e) : Exception::set(e);
   return e;
}
// FIXME: deprecated
ExceptionRef Exception::getLast()
{
   return Exception::get();
}
// FIXME: deprecated
bool Exception::hasLast()
{
   return Exception::isSet();
}
// FIXME: deprecated
void Exception::clearLast()
{
   Exception::clear();
}
// FIXME: deprecated
DynamicObject Exception::getLastAsDynamicObject()
{
   ExceptionRef e = Exception::get();
   return Exception::convertToDynamicObject(e);
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
   dyno["code"] = e->getCode();
   
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
   ExceptionRef e = new Exception(
      dyno["message"]->getString(),
      dyno["type"]->getString(),
      dyno["code"]->getInt32());
   
   if(dyno->hasMember("cause"))
   {
      ExceptionRef cause = convertToException(dyno["cause"]);
      e->setCause(cause);
   }
   
   if(dyno->hasMember("details"))
   {
      *(*e).mDetails = dyno["details"].clone();
   }
   
   return e;
}
