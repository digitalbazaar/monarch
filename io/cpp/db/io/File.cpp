/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include <sys/stat.h>

#include "db/io/File.h"

using namespace std;
using namespace db::io;

File::File()
{
}

File::File(const char* name)
{
   mName = name;
}

File::~File()
{
}

const string& File::getName()
{
   return mName;
}

bool File::exists()
{
   bool rval = false;
   struct stat s;
   int srval;
   
   srval = stat(mName.c_str(), &s);
   
   if(srval == 0)
   {
      rval = true;
   }
   
   return rval;
}
