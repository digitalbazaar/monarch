/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/FileOutputStream.h"

#include "monarch/rt/DynamicObject.h"

#include <cstring>

using namespace std;
using namespace monarch::io;
using namespace monarch::rt;

FileOutputStream::FileOutputStream(File& file, bool append, int bufferMode) :
   mFile(file),
   mAppend(append),
   mHandle(NULL),
   mBufferMode(bufferMode)
{
}

FileOutputStream::FileOutputStream(StdOutput out) :
   mFile((FileImpl*)NULL),
   mAppend(false),
   mHandle((out == StdOut) ? stdout : stderr)
{
}

FileOutputStream::~FileOutputStream()
{
   // close the handle if it is open
   FileOutputStream::close();
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
            "monarch.io.File.WriteError");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
      }
      else
      {
         rval = true;
      }
   }

   return rval;
}

bool FileOutputStream::flush()
{
   bool rval = false;

   if(ensureOpen())
   {
      fflush(mHandle);
   }

   return rval;
}

void FileOutputStream::close()
{
   // (mFile is null when using stdout/stderr)
   if(mHandle != NULL && !mFile.isNull())
   {
      // close the stream
      fclose(mHandle);
      mHandle = NULL;
   }
}

bool FileOutputStream::ensureOpen()
{
   bool rval = true;

   // try to open the file (mFile is null when using stdout/stderr)
   if(mHandle == NULL && !mFile.isNull())
   {
      mHandle = fopen(mFile->getAbsolutePath(), mAppend ? "ab" : "wb");
      if(mHandle == NULL)
      {
         ExceptionRef e = new Exception(
            "Could not open file stream.",
            "monarch.io.File.OpenFailed");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
         rval = false;
      }
      else
      {
         // set buffer mode (windows thinks 0 is an invalid argument for the
         // buffer size so we use the defined BUFSIZ minimum buffer size)
#ifdef WIN32
         if(setvbuf(mHandle, (char*)NULL, mBufferMode, BUFSIZ) != 0)
#else
         if(setvbuf(mHandle, (char*)NULL, mBufferMode, 0) != 0)
#endif
         {
            ExceptionRef e = new Exception(
               "Could not set file buffer mode.",
               "monarch.io.File.BufferModeFailure");
            e->getDetails()["path"] = mFile->getAbsolutePath();
            e->getDetails()["mode"] = mBufferMode;
            e->getDetails()["error"] = strerror(errno);
            Exception::set(e);
            rval = false;
         }
      }
   }

   return rval;
}
