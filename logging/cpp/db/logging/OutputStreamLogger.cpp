/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */

#include "db/logging/OutputStreamLogger.h"

#include <cstring>

using namespace db::io;
using namespace db::logging;

OutputStreamLogger::OutputStreamLogger(OutputStream* stream, bool cleanup) :
   Logger()
{
   setOutputStream(stream, cleanup, false);
}

OutputStreamLogger::~OutputStreamLogger()
{
   close();
}

void OutputStreamLogger::close()
{
   mLock.lock();
   {
      if(mStream != NULL)
      {
         mStream->close();
         
         if(mCleanup)
         {
            delete mStream;
            mStream = NULL;
         }
      }
   }
   mLock.unlock();
}

OutputStream* OutputStreamLogger::getOutputStream()
{
   return mStream;
}

void OutputStreamLogger::setOutputStream(OutputStream* os,
   bool cleanup, bool closeCurrent)
{
   mLock.lock();
   {
      if(closeCurrent)
      {
         close();
      }
      mStream = os;
      mCleanup = cleanup;
   }
   mLock.unlock();
}

void OutputStreamLogger::log(const char* message, size_t length)
{
   mLock.lock();
   {
      if(mStream != NULL)
      {
         mStream->write(message, length);
      }
   }
   mLock.unlock();
}

void OutputStreamLogger::flush()
{
   mLock.lock();
   {
      if(mStream != NULL)
      {
         mStream->flush();
      }
   }
   mLock.unlock();
}
