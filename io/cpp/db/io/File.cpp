/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/File.h"

#include <sys/stat.h>

using namespace db::io;

File::File()
{
   mName = strdup("");
}

File::File(const char* name)
{
   mName = strdup(name);
}

File::~File()
{
   delete [] mName;
}

bool File::exists()
{
   bool rval = false;
   
   struct stat s;
   int rc = stat(mName, &s);
   if(rc == 0)
   {
      rval = true;
   }
   else
   {
      // FIXME: add error handling
   }
   
   return rval;
}

bool File::remove()
{
   bool rval = false;
   
   int rc = ::remove(mName);
   if(rc == 0)
   {
      rval = true;
   }
   else
   {
      // FIXME: add error handling
   }
   
   return rval;
}

const char* File::getName()
{
   return mName;
}

off_t File::getLength()
{
   struct stat s;
   int rc = stat(mName, &s);
   if(rc != 0)
   {
      // FIXME: add error handling
   }
   
   return s.st_size;
}
