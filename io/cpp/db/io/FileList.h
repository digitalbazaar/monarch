/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
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
 * The Files added to this list will be deleted when this is destructed
 * unless otherwise specified.
 * 
 * @author Dave Longley
 */
class FileList : public db::util::UniqueList<File*>
{
protected:
   /**
    * True if the files in this list are heap-allocated and should be
    * cleaned up when this list is destructed.
    */
   bool mCleanupFiles;
   
public:
   /**
    * Creates a new FileList.
    * 
    * @param cleanupFiles true if the Files are heap-allocated and should
    *                     be freed by this list when it destructs, false if
    *                     not.
    */
   FileList(bool cleanupFiles);
   
   /**
    * Destructs this FileList.
    */
   virtual ~FileList();
};

} // end namespace io
} // end namespace db
#endif
