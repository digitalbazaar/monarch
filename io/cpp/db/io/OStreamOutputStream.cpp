/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/OStreamOutputStream.h"

#include <iostream>

using namespace std;
using namespace db::io;
using namespace db::rt;

// initialize standard output stream
OStreamOutputStream OStreamOutputStream::sStdoutStream(&cout);

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
   return &sStdoutStream;
}
