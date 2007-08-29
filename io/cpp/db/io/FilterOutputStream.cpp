/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FilterOutputStream.h"

using namespace db::io;

FilterOutputStream::FilterOutputStream(OutputStream* os, bool cleanup)
{
   mOutputStream = os;
   mCleanupOutputStream = cleanup;
}

FilterOutputStream::~FilterOutputStream()
{
   // cleanup wrapped output stream as appropriate
   if(mCleanupOutputStream && mOutputStream != NULL)
   {
      delete mOutputStream;
   }
}

bool FilterOutputStream::write(const char* b, unsigned int length)
{
   return mOutputStream->write(b, length);
}

void FilterOutputStream::close()
{
   mOutputStream->close();
}
