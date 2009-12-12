/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_io_FileList_H
#define monarch_io_FileList_H

#include "monarch/util/UniqueList.h"
#include "monarch/io/File.h"

namespace monarch
{
namespace io
{

/**
 * A FileList consists of a list of unique Files. The Files will be
 * compared on their equality operator: operator==.
 *
 * @author Dave Longley
 */
class FileList : public monarch::rt::Collectable< monarch::util::UniqueList<File> >
{
public:
   /**
    * Creates a new FileList.
    */
   FileList(
      monarch::util::UniqueList<File>* ptr = new monarch::util::UniqueList<File>()) :
      monarch::rt::Collectable< monarch::util::UniqueList<File> >(ptr) {};

   /**
    * Destructs this FileList.
    */
   virtual ~FileList() {};
};

} // end namespace io
} // end namespace monarch
#endif
