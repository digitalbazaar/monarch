/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/io/OStreamOutputStream.h"

#include "db/rt/Exception.h"

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
      ExceptionRef e = new Exception(
         "Could not write to ostream.", "db.io.IO");
      Exception::set(e);
   }

   return rval;
}