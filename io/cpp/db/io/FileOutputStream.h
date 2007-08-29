/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_FileOutputStream_H
#define db_io_FileOutputStream_H

#include "db/io/File.h"
#include "db/io/OutputStream.h"

#include <fstream>

namespace db
{
namespace io
{

/**
 * A FileOutputStream is used to write bytes to a file.
 * 
 * @author Dave Longley
 */
class FileOutputStream : public OutputStream
{
protected:
   /**
    * The File to write to.
    */
   File* mFile;
   
   /**
    * Whether or the file should be appended or overwritten.
    */
   bool mAppend;
   
   /**
    * The ofstream to write with.
    */
   std::ofstream mStream;
   
   /**
    * Ensures the file is open for writing.
    * 
    * @return true if the file is opened for writing, false if it cannot be
    *         opened.
    */
   bool ensureOpen();

public:
   /**
    * Creates a new FileOutputStream that opens the passed File for writing.
    * 
    * @param file the File to write to.
    * @param append true to append to the File if it exists, false to overwrite.
    */
   FileOutputStream(File* file, bool append = false);
   
   /**
    * Destructs this FileOutputStream.
    */
   virtual ~FileOutputStream();
   
   /**
    * Writes some bytes to the stream.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred. 
    */
   virtual bool write(const char* b, unsigned int length);
   
   /**
    * Closes the stream.
    */
   virtual void close();
};

} // end namespace io
} // end namespace db
#endif
