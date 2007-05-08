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

bool FilterInputStream::read(char& b) throw(IOException)
{
   return mInputStream->read(b);
}

int FilterInputStream::read(char* b, unsigned int length) throw(IOException)
{
   return mInputStream->read(b, length);
}

int FilterInputStream::peek(char* b, unsigned int length) throw(IOException)
{
   return mInputStream->peek(b, length);
}

unsigned long FilterInputStream::skip(unsigned long count)
throw(IOException)
{
   return mInputStream->skip(count);
}

void FilterInputStream::close() throw(IOException)
{
   mInputStream->close();
}
