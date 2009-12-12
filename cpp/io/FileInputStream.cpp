/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/FileInputStream.h"

#include "monarch/io/FileFunctions.h"
#include "monarch/rt/DynamicObject.h"

#include <cstring>
#include <cstdlib>

using namespace std;
using namespace monarch::io;
using namespace monarch::rt;

FileInputStream::FileInputStream(File& file) :
   mFile(file),
   mHandle(NULL)
{
}

FileInputStream::~FileInputStream()
{
   // close the handle if it is open
   FileInputStream::close();
}

bool FileInputStream::ensureOpen()
{
   bool rval = true;

   // try to open the file
   if(mHandle == NULL)
   {
      if(!mFile->exists())
      {
         ExceptionRef e = new Exception(
            "Could not open file.",
            "monarch.io.File.NotFound");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         Exception::set(e);
         rval = false;
      }
      else if(!mFile->isReadable())
      {
         ExceptionRef e = new Exception(
            "Could not open file.",
            "monarch.io.File.AccessDenied");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         Exception::set(e);
         rval = false;
      }
      else
      {
         mHandle = fopen(mFile->getAbsolutePath(), "rb");
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
      }
   }

   return rval;
}

int FileInputStream::read(char* b, int length)
{
   int rval = -1;

   if(ensureOpen())
   {
      rval = 0;

      // do read
      int count = fread(b, 1, length, mHandle);
      if(count != length)
      {
         // check for an error
         if(ferror(mHandle) != 0)
         {
            ExceptionRef e = new Exception(
               "Could not read file.",
               "monarch.io.File.ReadError");
            e->getDetails()["path"] = mFile->getAbsolutePath();
            e->getDetails()["error"] = strerror(errno);
            Exception::set(e);
            rval = -1;
         }
      }

      if(rval != -1)
      {
         // return number of bytes read
         rval = count;
      }
   }

   return rval;
}

long long FileInputStream::skip(long long count)
{
   long long rval = -1;

   if(ensureOpen())
   {
      bool success;

      // store current position and the end position
      size_t curr = ftell(mHandle);
      success = (fseek(mHandle, 0, SEEK_END) == 0);
      size_t end = ftell(mHandle);
      success = success && (fseek(mHandle, curr, SEEK_SET) == 0);

      rval = count;
      if(rval > 0 && curr < end)
      {
         // do not skip past EOF
         if(rval > (end - curr))
         {
            rval = end - curr;
         }

         // skip from current offset
         success = success && (fseek(mHandle, rval, SEEK_CUR) == 0);
      }

      if(!success)
      {
         ExceptionRef e = new Exception(
            "Could not read file.",
            "monarch.io.File.ReadError");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         e->getDetails()["error"] = strerror(errno);
         Exception::set(e);
         rval = -1;
      }
   }

   return rval;
}

int FileInputStream::readLine(string& line, char delimiter)
{
   int rval = -1;

   if(ensureOpen())
   {
      rval = 0;

      // feof returns non-zero when EOF
      if(feof(mHandle) == 0)
      {
         // get line
         char* data = NULL;
         size_t size = 0;
         ssize_t length = getdelim(&data, &size, delimiter, mHandle);
         if(length == -1)
         {
            ExceptionRef e = new Exception(
               "Could not read file.",
               "monarch.io.File.ReadError");
            e->getDetails()["path"] = mFile->getAbsolutePath();
            e->getDetails()["error"] = strerror(errno);
            Exception::set(e);
            rval = -1;
         }
         else
         {
            // line was read
            rval = 1;
            if(data[length - 1] == delimiter)
            {
               // do not include delimiter
               line.assign(data, length - 1);
            }
            else
            {
               line.assign(data, length);
            }
            free(data);
         }
      }
   }

   return rval;
}

void FileInputStream::close()
{
   if(mHandle != NULL)
   {
      // close the stream
      fclose(mHandle);
      mHandle = NULL;
   }
}
