/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FilterInputStream.h"

using namespace db::io;

FilterInputStream::FilterInputStream(InputStream* is, bool cleanup)
{
   mInputStream = is;
   mCleanupInputStream = cleanup;
}

FilterInputStream::~FilterInputStream()
{
   // cleanup wrapped input stream as appropriate
   if(mCleanupInputStream && mInputStream != NULL)
   {
      delete mInputStream;
   }
}

int FilterInputStream::read(char* b, int length)
{
   return mInputStream->read(b, length);
}

int FilterInputStream::peek(char* b, int length, bool block)
{
   return mInputStream->peek(b, length, block);
}

long long FilterInputStream::skip(long long count)
{
   return InputStream::skip(count);
}

void FilterInputStream::close()
{
   mInputStream->close();
}
