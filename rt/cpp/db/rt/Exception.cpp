/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/Exception.h"
#include "db/rt/Thread.h"

#include <string>

using namespace db::rt;

Exception::Exception(const char* message, const char* type, int code)
{
   const char* str = (message == NULL) ? "" : message;
   mMessage = new char[strlen(message) + 1];
   strcpy(mMessage, message);
   
   str = (type == NULL) ? "" : type;
   mType = new char[strlen(type) + 1];
   strcpy(mType, type);
   
   mCode = code;
   
   mCause = NULL;
   mCleanupCause = false;
}

Exception::~Exception()
{
   delete [] mMessage;
   delete [] mType;
   
   if(mCause != NULL && mCleanupCause)
   {
      delete mCause;
   }
}

void Exception::setMessage(const char* message)
{
   const char* str = (message == NULL) ? "" : message;
   delete [] mMessage;
   mMessage = new char[strlen(str) + 1];
   strcpy(mMessage, str);
}

void Exception::setType(const char* type)
{
   const char* str = (type == NULL) ? "" : type;
   delete [] mType;
   mType = new char[strlen(str) + 1];
   strcpy(mType, str);
}

void Exception::setCode(int code)
{
   mCode = code;
}

const char* Exception::getMessage()
{
   return mMessage;
}

const char* Exception::getType()
{
   return mType;
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
