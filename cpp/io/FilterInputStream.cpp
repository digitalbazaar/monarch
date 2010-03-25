/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/FilterInputStream.h"

using namespace monarch::io;

FilterInputStream::FilterInputStream(InputStream* is, bool cleanup) :
   mInputStream(NULL),
   mCleanupInputStream(false)
{
   setInputStream(is, cleanup);
}

FilterInputStream::~FilterInputStream()
{
   setInputStream(NULL, false);
}

void FilterInputStream::setInputStream(InputStream* is, bool cleanup)
{
   // cleanup wrapped input stream as appropriate
   if(mCleanupInputStream && mInputStream != NULL)
   {
      delete mInputStream;
   }
   mInputStream = is;
   mCleanupInputStream = cleanup;
}

inline int FilterInputStream::read(char* b, int length)
{
   return mInputStream->read(b, length);
}

inline int FilterInputStream::peek(char* b, int length, bool block)
{
   return mInputStream->peek(b, length, block);
}

inline int64_t FilterInputStream::skip(int64_t count)
{
   return InputStream::skip(count);
}

inline void FilterInputStream::close()
{
   mInputStream->close();
}
