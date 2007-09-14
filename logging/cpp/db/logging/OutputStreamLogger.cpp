/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/logging/OutputStreamLogger.h"

using namespace db::io;
using namespace db::logging;

OutputStreamLogger::OutputStreamLogger(const char* name, Level level,
   OutputStream* stream, bool cleanup) :
   Logger(name, level)
{
   setOutputStream(stream, cleanup);
}

OutputStreamLogger::~OutputStreamLogger()
{
   close();
}

void OutputStreamLogger::close()
{
   if(mCleanup && mStream != NULL)
   {
      mStream->close();
      delete mStream;
   }
}

OutputStream* OutputStreamLogger::getOutputStream()
{
   return mStream;
}

void OutputStreamLogger::setOutputStream(OutputStream* os, bool cleanup)
{
   lock();
   close();
   mStream = os;
   mCleanup = cleanup;
   unlock();
}

void OutputStreamLogger::log(const char* message)
{
   if(mStream != NULL)
   {
      mStream->write(message, strlen(message));
   }
}
