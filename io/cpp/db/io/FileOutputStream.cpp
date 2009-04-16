/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/io/FileOutputStream.h"

#include "db/rt/DynamicObject.h"

#include <cstring>

using namespace std;
using namespace db::io;
using namespace db::rt;

FileOutputStream::FileOutputStream(File& file, bool append) :
   mFile(file),
   mAppend(append),
   mHandle(NULL)
{
}

FileOutputStream::~FileOutputStream()
{
   // close the handle if it is open
   FileOutputStream::close();
}

bool FileOutputStream::ensureOpen()
{
   bool rval = true;
   
   // try to open the file
   if(mHandle == NULL)
   {
      mHandle = fopen(mFile->getAbsolutePath(), mAppend ? "ab" : "wb");
      if(mHandle == NULL)
      {
         ExceptionRef e = new Exception(
            "Could not open file stream.",
            "db.io.File.OpenFailed");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         e->getDetails()["error"] = strerror(errno);
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
      if((int)fwrite(b, 1, length, mHandle) != length)
      {
         // error
         ExceptionRef e = new Exception(
            "Could not write to file.",
            "db.io.File.WriteError");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         e->getDetails()["error"] = strerror(errno);
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
   if(mHandle != NULL)
   {
      // close the stream
      fclose(mHandle);
      mHandle = NULL;
   }
}
