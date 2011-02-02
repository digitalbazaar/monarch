/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/FilterOutputStream.h"

using namespace monarch::io;

FilterOutputStream::FilterOutputStream(OutputStream* os, bool cleanup) :
   mOutputStream(os),
   mCleanupOutputStream(cleanup)
{
}

FilterOutputStream::~FilterOutputStream()
{
   if(mCleanupOutputStream)
   {
      delete mOutputStream;
   }
}

inline bool FilterOutputStream::write(const char* b, int length)
{
   return mOutputStream->write(b, length);
}

inline bool FilterOutputStream::flush()
{
   return mOutputStream->flush();
}

inline bool FilterOutputStream::finish()
{
   return mOutputStream->finish();
}

inline void FilterOutputStream::close()
{
   mOutputStream->close();
}
