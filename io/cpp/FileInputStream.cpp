/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "FileInputStream.h"

using namespace db::io;

FileInputStream::FileInputStream(File* file) throw(IOException)
{
   // store file
   mFile = file;
   
   // try to open the file
   mStream.open(file->getName().c_str());
   if(!mStream.is_open())
   {
      throw IOException("Could not open file '" + mFile->getName() + "'!");
   }
}

FileInputStream::~FileInputStream()
{
}

bool FileInputStream::read(char& b) throw(IOException)
{
   mStream.read(&b, 1);
   if(mStream.fail())
   {
      throw IOException("Could not read from file '" + mFile->getName() + "'!");
   }
   
   return !mStream.eof();
}

int FileInputStream::read(
   char* b, unsigned int offset, unsigned int length)
throw(IOException)
{
   int rval = -1;
   
   mStream.read(b + offset, length);
   if(mStream.fail())
   {
      throw IOException("Could not read from file '" + mFile->getName() + "'!");
   }
   
   if(mStream.gcount() > 0)
   {
      rval = mStream.gcount();
   }
   
   return rval;
}

void FileInputStream::close() throw(IOException)
{
   // close the stream
   mStream.close();
}
