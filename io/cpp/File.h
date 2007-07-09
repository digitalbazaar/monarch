/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_File_H
#define db_io_File_H

#include "Object.h"

#include <string>

namespace db
{
namespace io
{

/**
 * A File represents a file or directory on a disk.
 * 
 * FIXME: this class needs to be filled out with simple file meta-data
 * retrieving methods
 * 
 * @author Dave Longley
 */
class File : public virtual db::rt::Object
{
protected:
   /**
    * Stores the name of this file.
    */
   std::string mName;

public:
   /**
    * Creates a new File.
    */
   File();
   
   /**
    * Creates a new File with the specified name.
    * 
    * @param name the name of the file.
    */
   File(const std::string& name);
   
   /**
    * Destructs this File.
    */
   virtual ~File();
   
   /**
    * Gets the name of this File.
    * 
    * @return the name of this File.
    */
   virtual const std::string& getName();
};

} // end namespace io
} // end namespace db
#endif
