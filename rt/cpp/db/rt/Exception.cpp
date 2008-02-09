/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/Exception.h"

#include "db/rt/Thread.h"
#include "db/rt/DynamicObject.h"

using namespace db::rt;

Exception::Exception(const char* message, const char* type, int code)
{
   mMessage = strdup((message == NULL) ? "" : message);
   mType = strdup((type == NULL) ? "" : type);
   mCode = code;
   mCause = new ExceptionRef(NULL);
}

Exception::~Exception()
{
   free(mMessage);
   free(mType);
   delete mCause;
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

ExceptionRef& Exception::setLast(ExceptionRef& e, bool caused)
{
   Thread::setException(e, caused);
   return e;
}

ExceptionRef Exception::getLast()
{
   return Thread::getException();
}

bool Exception::hasLast()
{
   return Thread::hasException();
}

void Exception::clearLast()
{
   Thread::clearException();
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
   
   return e;
}
