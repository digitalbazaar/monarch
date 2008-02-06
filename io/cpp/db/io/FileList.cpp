/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FileList.h"

using namespace db::io;
using namespace db::rt;
using namespace db::util;

FileList::FileList(bool cleanupFiles)
{
   mCleanupFiles = cleanupFiles;
}

FileList::~FileList()
{
   if(mCleanupFiles)
   {
      db::rt::IteratorRef<File*> i = getIterator();
      while(i->hasNext())
      {
         // clean up File
         delete i->next();
      }
   }
}
