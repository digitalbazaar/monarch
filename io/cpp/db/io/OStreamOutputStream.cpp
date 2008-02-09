/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/OStreamOutputStream.h"

using namespace std;
using namespace db::io;
using namespace db::rt;

OStreamOutputStream::OStreamOutputStream(ostream* stream)
{
   // store ostream
   mStream = stream;
}

OStreamOutputStream::~OStreamOutputStream()
{
}

bool OStreamOutputStream::write(const char* b, int length)
{
   bool rval = false;
   
   // do write
   mStream->write(b, length);
   
   // see if a failure has occurred
   if(!(rval = !mStream->fail()))
   {
      ExceptionRef e = new IOException("Could not write to ostream!");
      Exception::setLast(e, false);
   }
   
   return rval;
}
