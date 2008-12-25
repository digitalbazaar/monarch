/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FileInputStream.h"

#include "db/rt/DynamicObject.h"

#include <cstring>

using namespace std;
using namespace db::io;
using namespace db::rt;

FileInputStream::FileInputStream(File& file) :
   mFile(file)
{
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
         mStream.open(mFile->getAbsolutePath(), ios::in | ios::binary);
         if(!mStream.is_open())
         {
            ExceptionRef e = new Exception(
               "Could not open file stream.",
               "db.io.File.OpenFailed");
            e->getDetails()["path"] = mFile->getAbsolutePath();
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
      
      if(!mStream.eof())
      {
         // do read
         mStream.read(b, length);
         
         // see if a failure other than EOF occurred
         if(mStream.fail() && !mStream.eof())
         {
            ExceptionRef e = new Exception(
               "Could not read file.",
               "db.io.File.ReadError");
            e->getDetails()["path"] = mFile->getAbsolutePath();
            Exception::setLast(e, false);
            rval = -1;
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

long long FileInputStream::skip(long long count)
{
   long long rval = -1;
   
   if(ensureOpen())
   {
      // store current position and the end position
      streampos curr = mStream.tellg();
      mStream.seekg(0, ios::end);
      streampos end = mStream.tellg();
      mStream.seekg(curr, ios::beg);
      
      rval = count;
      if(rval > 0 && curr < end)
      {
         // do not skip past EOF
         if(rval > (end - curr))
         {
            rval = end - curr;
         }
         
         // skip from current offset
         mStream.seekg(rval, ios::cur);
         curr = mStream.tellg();
      }
      
      // see if a failure other than EOF occurred
      if(mStream.fail() && !mStream.eof())
      {
         ExceptionRef e = new Exception(
            "Could not read file.",
            "db.io.File.ReadError");
         e->getDetails()["path"] = mFile->getAbsolutePath();
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
      
      if(!mStream.eof())
      {
         // get line
         getline(mStream, line, delimiter);
         
         // see if a failure other than EOF occurred
         if(mStream.fail() && !mStream.eof())
         {
            ExceptionRef e = new Exception(
               "Could not read file.",
               "db.io.File.ReadError");
            e->getDetails()["path"] = mFile->getAbsolutePath();
            Exception::setLast(e, false);
            rval = -1;
         }
         else
         {
            // line was read
            rval = 1;
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
