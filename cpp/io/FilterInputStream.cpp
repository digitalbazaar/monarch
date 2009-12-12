/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/FilterInputStream.h"

using namespace db::io;

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

inline long long FilterInputStream::skip(long long count)
{
   return InputStream::skip(count);
}

inline void FilterInputStream::close()
{
   mInputStream->close();
}
