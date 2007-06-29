/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "FileInputStream.h"

using namespace std;
using namespace db::io;
using namespace db::rt;

FileInputStream::FileInputStream(File* file)
{
   // store file
   mFile = file;
}

FileInputStream::~FileInputStream()
{
   // close the ifstream if it is open
   if(mStream.is_open())
   {
      mStream.close();
   }
}

bool FileInputStream::ensureOpen()
{
   bool rval = true;
   
   // try to open the file
   if(!mStream.is_open())
   {
      mStream.open(mFile->getName().c_str(), ios::in | ios::binary);
      if(!mStream.is_open())
      {
         rval = false;
         Thread::setException(new IOException(
            "Could not open file '" + mFile->getName() + "'!"));
      }
   }
   
   return rval;
}

int FileInputStream::read(char* b, unsigned int length)
{
   int rval = -1;
   
   if(ensureOpen() && !mStream.eof())
   {
      // do read
      mStream.read(b, length);
      
      // see if a failure other than EOF occurred
      if(mStream.fail() && !mStream.eof())
      {
         Thread::setException(new IOException(
            "Could not read from file '" + mFile->getName() + "'!"));
      }
      else if(mStream.gcount() > 0)
      {
         // get the number of bytes read
         rval = mStream.gcount();
      }
   }
   
   return rval;
}

void FileInputStream::close()
{
   // close the stream
   mStream.close();
}
