/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "FileOutputStream.h"

using namespace std;
using namespace db::io;
using namespace db::rt;

FileOutputStream::FileOutputStream(File* file, bool append)
{
   // store file and append mode
   mFile = file;
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
         mStream.open(mFile->getName().c_str(),
            ios::out | ios::app | ios::binary);
      }
      else
      {
         mStream.open(mFile->getName().c_str(),
            ios::out | ios::trunc | ios::binary);
      }
      
      if(!mStream.is_open())
      {
         rval = false;
         string msg = "Could not open file '" + mFile->getName() + "'!";
         Exception::setLast(new IOException(msg.c_str()));
      }
   }
   
   return rval;
}

bool FileOutputStream::write(const char* b, unsigned int length)
{
   bool rval = false;
   
   if(ensureOpen())
   {
      // do write
      mStream.write(b, length);
      
      // see if a failure has occurred
      if(mStream.fail())
      {
         string msg = "Could not write to file '" + mFile->getName() + "'!";
         Exception::setLast(new IOException(msg.c_str()));
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
