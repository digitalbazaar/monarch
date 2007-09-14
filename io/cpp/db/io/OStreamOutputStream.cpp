/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include <iostream>

#include "db/io/OStreamOutputStream.h"

using namespace std;
using namespace db::io;
using namespace db::rt;

OStreamOutputStream* OStreamOutputStream::sStdoutStream =
   new OStreamOutputStream(&cout);

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
   if(mStream->fail())
   {
      Exception::setLast(new IOException("Could not write to ostream!"));
   }
   else
   {
      rval = true;
   }
   
   return rval;
}

OStreamOutputStream* OStreamOutputStream::getStdoutStream()
{
   return sStdoutStream;
}
