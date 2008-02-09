/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FileInputStream.h"

using namespace std;
using namespace db::io;
using namespace db::rt;

FileInputStream::FileInputStream(File* file, bool cleanup)
{
   // store file, clean up setting
   mFile = file;
   mCleanupFile = cleanup;
}

FileInputStream::~FileInputStream()
{
   // close the ifstream if it is open
   if(mStream.is_open())
   {
      mStream.close();
   }
   
   // clean up file
   if(mCleanupFile)
   {
      delete mFile;
   }
}

bool FileInputStream::ensureOpen()
{
   bool rval = true;
   
   // try to open the file
   if(!mStream.is_open())
   {
      mStream.open(mFile->getName(), ios::in | ios::binary);
      if(!mStream.is_open())
      {
         rval = false;
         char temp[strlen(mFile->getName()) + 30];
         sprintf(temp, "Could not open file '%s'!", mFile->getName());
         ExceptionRef e = new IOException(temp);
         Exception::setLast(e, false);
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
      
      if(!mStream.eof())
      {
         // do read
         mStream.read(b, length);
         
         // see if a failure other than EOF occurred
         if(mStream.fail() && !mStream.eof())
         {
            rval = -1;
            char temp[strlen(mFile->getName()) + 40];
            sprintf(temp, "Could not read from file '%s'!", mFile->getName());
            ExceptionRef e = new IOException(temp);
            Exception::setLast(e, false);
         }
         else if(mStream.gcount() > 0)
         {
            // get the number of bytes read
            rval = mStream.gcount();
         }
      }
   }
   
   return rval;
}

void FileInputStream::close()
{
   // close the stream
   mStream.close();
}
