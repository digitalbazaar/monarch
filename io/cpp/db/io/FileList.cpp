/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FileList.h"

using namespace db::io;
using namespace db::util;

FileList::FileList(bool cleanupFiles)
{
   mCleanupFiles = cleanupFiles;
}

FileList::~FileList()
{
   if(mCleanupFiles)
   {
      Iterator<File*>* i = getIterator();
      while(i->hasNext())
      {
         // clean up File
         delete i->next();
      }
      delete i;
   }
}
