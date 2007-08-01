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
         string msg = "Could not open file '" + mFile->getName() + "'!";
         Exception::setLast(new IOException(msg.c_str()));
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
         string msg = "Could not read from file '" + mFile->getName() + "'!";
         Exception::setLast(new IOException(msg.c_str()));
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
