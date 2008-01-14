/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/logging/OutputStreamLogger.h"

using namespace db::io;
using namespace db::logging;

OutputStreamLogger::OutputStreamLogger(
   Level level, OutputStream* stream, bool cleanup) :
   Logger(level)
{
   setOutputStream(stream, cleanup, false);
}

OutputStreamLogger::~OutputStreamLogger()
{
   close();
}

void OutputStreamLogger::close()
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

OutputStream* OutputStreamLogger::getOutputStream()
{
   return mStream;
}

void OutputStreamLogger::setOutputStream(OutputStream* os,
   bool cleanup, bool closeCurrent)
{
   lock();
   {
      if(closeCurrent)
      {
         close();
      }
      mStream = os;
      mCleanup = cleanup;
   }
   unlock();
}

void OutputStreamLogger::log(const char* message)
{
   if(mStream != NULL)
   {
      mStream->write(message, strlen(message));
   }
}
