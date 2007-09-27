/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/Exception.h"
#include "db/rt/Thread.h"

#include <string>

using namespace db::rt;

Exception::Exception(const char* message, const char* code)
{
   if(message == NULL)
   {
      mMessage = new char[1];
      mMessage[0] = 0;
   }
   else
   {
      mMessage = new char[strlen(message) + 1];
      strcpy(mMessage, message);
   }
   
   if(code == NULL)
   {
      mCode = new char[1];
      mCode[0] = 0;
   }
   else
   {
      mCode = new char[strlen(code) + 1];
      strcpy(mCode, code);
   }
   
   mCause = NULL;
   mCleanupCause = false;
}

Exception::~Exception()
{
   delete [] mMessage;
   delete [] mCode;
   
   if(mCause != NULL && mCleanupCause)
   {
      delete mCause;
   }
}

void Exception::setMessage(const char* message)
{
   delete [] mMessage;
   
   if(message == NULL)
   {
      mMessage = new char[1];
      mMessage[0] = 0;
   }
   else
   {
      mMessage = new char[strlen(message) + 1];
      strcpy(mMessage, message);
   }
}

void Exception::setCode(const char* code)
{
   delete [] mCode;
   
   if(code == NULL)
   {
      mCode = new char[1];
      mCode[0] = 0;
   }
   else
   {
      mCode = new char[strlen(code) + 1];
      strcpy(mCode, code);
   }
}

const char* Exception::getMessage()
{
   return mMessage;
}

const char* Exception::getCode()
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

Exception* Exception::setLast(Exception* e)
{
   Thread::setException(e);
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

void Exception::clearLast()
{
   Thread::clearException();
}
