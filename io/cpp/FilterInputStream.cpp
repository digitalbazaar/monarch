/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "FilterInputStream.h"

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

int FilterInputStream::read(char* b, unsigned int length)
{
   return mInputStream->read(b, length);
}

int FilterInputStream::peek(char* b, unsigned int length)
{
   return mInputStream->peek(b, length);
}

long FilterInputStream::skip(long count)
{
   return mInputStream->skip(count);
}

void FilterInputStream::close()
{
   mInputStream->close();
}
