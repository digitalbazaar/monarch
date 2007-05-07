/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "FileInputStream.h"

using namespace std;
using namespace db::io;

FileInputStream::FileInputStream(File* file) throw(IOException)
{
   // store file
   mFile = file;
   
   // try to open the file
   mStream.open(file->getName().c_str(), ios::in | ios::binary);
   if(!mStream.is_open())
   {
      throw IOException("Could not open file '" + mFile->getName() + "'!");
   }
}

FileInputStream::~FileInputStream()
{
   // close the ifstream if it is open
   if(mStream.is_open())
   {
      mStream.close();
   }
}

bool FileInputStream::read(char& b) throw(IOException)
{
   bool rval = false;
   
   if(!mStream.eof())
   {
      // do read
      mStream.read(&b, 1);
      
      // see if a failure other than EOF occurred
      if(mStream.fail() && !mStream.eof())
      {
         throw IOException(
            "Could not read from file '" + mFile->getName() + "'!");
      }
      
      // read successful, not end of stream yet
      rval = true;
   }
   
   return rval;
}

int FileInputStream::read(
   char* b, unsigned int offset, unsigned int length)
throw(IOException)
{
   int rval = -1;
   
   if(!mStream.eof())
   {
      // do read
      mStream.read(b + offset, length);
      
      // see if a failure other than EOF occurred
      if(mStream.fail() && !mStream.eof())
      {
         throw IOException(
            "Could not read from file '" + mFile->getName() + "'!");
      }
      
      // get the number of bytes read
      if(mStream.gcount() > 0)
      {
         rval = mStream.gcount();
      }
   }
   
   return rval;
}

void FileInputStream::close() throw(IOException)
{
   // close the stream
   mStream.close();
}
