/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "FilterOutputStream.h"

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

void FilterOutputStream::write(const char& b) throw(IOException)
{
   mOutputStream->write(b);
}

void FilterOutputStream::write(
   const char* b, unsigned int offset, unsigned int length) throw(IOException)
{
   mOutputStream->write(b, offset, length);
}

void FilterOutputStream::close()
{
   mOutputStream->close();
}
