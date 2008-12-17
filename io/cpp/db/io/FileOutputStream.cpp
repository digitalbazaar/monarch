/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FileOutputStream.h"

#include "db/rt/DynamicObject.h"

#include <cstring>

using namespace std;
using namespace db::io;
using namespace db::rt;

FileOutputStream::FileOutputStream(File& file, bool append) :
   mFile(file)
{
   // store append mode
   mAppend = append;
}

FileOutputStream::~FileOutputStream()
{
}

bool FileOutputStream::ensureOpen()
{
   bool rval = true;
   
   // try to open the file
   if(!mStream.is_open())
   {
      if(mAppend)
      {
         mStream.open(
            mFile->getAbsolutePath(),
            ios::out | ios::app | ios::binary);
      }
      else
      {
         mStream.open(
            mFile->getAbsolutePath(),
            ios::out | ios::trunc | ios::binary);
      }
      
      if(!mStream.is_open())
      {
         ExceptionRef e = new Exception(
            "Could not open file.",
            "db.io.File.OpenFailed");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         Exception::setLast(e, false);
         rval = false;
      }
   }
   
   return rval;
}

bool FileOutputStream::write(const char* b, int length)
{
   bool rval = false;
   
   if(ensureOpen())
   {
      // do write
      mStream.write(b, length);
      
      // see if a failure has occurred
      if(mStream.fail())
      {
         ExceptionRef e = new Exception(
            "Could not write to file.",
            "db.io.File.WriteError");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         Exception::setLast(e, false);
      }
      else
      {
         rval = true;
      }
   }
   
   return rval;
}

void FileOutputStream::close()
{
   // close the stream
   mStream.close();
}
