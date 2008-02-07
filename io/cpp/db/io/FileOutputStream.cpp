/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FileOutputStream.h"

using namespace std;
using namespace db::io;
using namespace db::rt;

FileOutputStream::FileOutputStream(File* file, bool append, bool cleanup)
{
   // store file and append mode
   mFile = file;
   mAppend = append;
   mCleanupFile = cleanup;
}

FileOutputStream::~FileOutputStream()
{
   if(mCleanupFile)
   {
      delete mFile;
   }
}

bool FileOutputStream::ensureOpen()
{
   bool rval = true;
   
   // try to open the file
   if(!mStream.is_open())
   {
      if(mAppend)
      {
         mStream.open(mFile->getName(), ios::out | ios::app | ios::binary);
      }
      else
      {
         mStream.open(mFile->getName(), ios::out | ios::trunc | ios::binary);
      }
      
      if(!mStream.is_open())
      {
         rval = false;
         char temp[strlen(mFile->getName()) + 30];
         sprintf(temp, "Could not open file '%s'!", mFile->getName());
         ExceptionRef e = new IOException(temp);
         Exception::setLast(e);
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
         char temp[strlen(mFile->getName()) + 40];
         sprintf(temp, "Could not write to file '%s'!", mFile->getName());
         ExceptionRef e = new IOException(temp);
         Exception::setLast(e);
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
