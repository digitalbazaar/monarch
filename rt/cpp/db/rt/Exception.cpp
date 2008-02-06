/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/Exception.h"
#include "db/rt/Thread.h"

#include <string>

using namespace db::rt;

Exception::Exception(const char* message, const char* type, int code)
{
   mMessage = strdup((message == NULL) ? "" : message);
   mType = strdup((type == NULL) ? "" : type);
   mCode = code;
   
   mCause = NULL;
   mCleanupCause = false;
}

Exception::~Exception()
{
   free(mMessage);
   free(mType);
   
   if(mCause != NULL && mCleanupCause)
   {
      delete mCause;
   }
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

void Exception::setCause(Exception* cause, bool cleanup)
{
   mCause = cause;
   mCleanupCause = cleanup;
}

Exception* Exception::getCause()
{
   return mCause;
}

Exception* Exception::setLast(Exception* e, bool cleanup)
{
   Thread::setException(e, cleanup);
   return e;
}

Exception* Exception::getLast()
{
   return Thread::getException();
}

bool Exception::hasLast()
{
   return Thread::hasException();
}

void Exception::clearLast(bool cleanup)
{
   Thread::clearException(cleanup);
}
