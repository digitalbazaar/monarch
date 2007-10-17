/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_File_H
#define db_io_File_H

#include "db/rt/Object.h"

#include <string>

namespace db
{
namespace io
{

// forward declare file list
class FileList;

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
   char* mName;
   
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
   File(const char* name);
   
   /**
    * Destructs this File.
    */
   virtual ~File();
   
   /**
    * Returns true if this File is equal to the passed one. Two Files are
    * only equal if their names are the same and they are the same type,
    * meaning they are both regular files, both directories, or both
    * symbolic links.
    * 
    * @param rhs the File to compare to this one.
    * 
    * @return true if this File is equal to the passed one, false if not.
    */
   bool operator==(const File& rhs);
   
   /**
    * Determines whether or not this file physically exists.
    * 
    * @return true if this file exists, false if not.
    */
   virtual bool exists();
   
   /**
    * Deletes this file, if it exists.
    * 
    * @return true if this file was deleted, false if not.
    */
   virtual bool remove();
   
   /**
    * Gets the name of this File.
    * 
    * @return the name of this File.
    */
   virtual const char* getName() const;
   
   /**
    * Gets the length of this File.
    * 
    * @return the length of this File.
    */
   virtual off_t getLength();
   
   /**
    * Returns true if this File is a regular file, false if it is not. If it
    * is not, then it may be a directory or a symbolic link. 
    * 
    * @return true if this File is a regular file, false if not.
    */
   virtual bool isFile();
   
   /**
    * Returns true if this File is a directory, false if it is not. If it
    * is not, then it may be a regular file or a symbolic link. 
    * 
    * @return true if this File is a directory, false if not.
    */
   virtual bool isDirectory();
   
   /**
    * Returns true if this File is a symbolic link, false if it is not. If it
    * is not, then it may be a regular file or a directory. 
    * 
    * @return true if this File is a symbolic link, false if not.
    */
   virtual bool isSymbolicLink();
   
   /**
    * Populates a list with all of the Files in this File, if this File is
    * a directory. Each File added to the list will be heap-allocated, and it
    * is assumed that the passed list will manage their memory.
    * 
    * @param files the FileList to populate.
    */
   virtual void listFiles(FileList* files);
};

} // end namespace io
} // end namespace db
#endif
