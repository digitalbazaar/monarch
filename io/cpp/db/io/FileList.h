/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_FileList_H
#define db_io_FileList_H

#include "db/util/UniqueList.h"
#include "db/io/File.h"

namespace db
{
namespace io
{

/**
 * A FileList consists of a list of unique Files. The Files will be
 * compared on their equality operator: operator==.
 * 
 * @author Dave Longley
 */
class FileList : public db::rt::Collectable< db::util::UniqueList<File> >
{
public:
   /**
    * Creates a new FileList.
    */
   FileList(
      db::util::UniqueList<File>* ptr = new db::util::UniqueList<File>()) :
      db::rt::Collectable< db::util::UniqueList<File> >(ptr) {};
   
   /**
    * Destructs this FileList.
    */
   virtual ~FileList() {};
};

} // end namespace io
} // end namespace db
#endif
