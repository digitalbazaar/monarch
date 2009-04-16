/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/io/FileInputStream.h"

#include "db/rt/DynamicObject.h"
#include "db/io/FileFunctions.h"

#include <cstring>
#include <cstdlib>

using namespace std;
using namespace db::io;
using namespace db::rt;

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
            "db.io.File.NotFound");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         Exception::setLast(e, false);
         rval = false;
      }
      else if(!mFile->isReadable())
      {
         ExceptionRef e = new Exception(
            "Could not open file.",
            "db.io.File.AccessDenied");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         Exception::setLast(e, false);
         rval = false;
      }
      else
      {
         mHandle = fopen(mFile->getAbsolutePath(), "rb");
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
         // check for an error other than EOF
         if((count != 0 || feof(mHandle) == 0) && ferror(mHandle) != 0)
         {
            ExceptionRef e = new Exception(
               "Could not read file.",
               "db.io.File.ReadError");
            e->getDetails()["path"] = mFile->getAbsolutePath();
            e->getDetails()["error"] = strerror(errno);
            Exception::setLast(e, false);
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
            "db.io.File.ReadError");
         e->getDetails()["path"] = mFile->getAbsolutePath();
         e->getDetails()["error"] = strerror(errno);
         Exception::setLast(e, false);
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
         size_t length = 0;
         if(getdelim(&data, &length, delimiter, mHandle) == -1)
         {
            ExceptionRef e = new Exception(
               "Could not read file.",
               "db.io.File.ReadError");
            e->getDetails()["path"] = mFile->getAbsolutePath();
            e->getDetails()["error"] = strerror(errno);
            Exception::setLast(e, false);
            rval = -1;
         }
         else
         {
            // line was read
            rval = 1;
            line.assign(data, length);
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
