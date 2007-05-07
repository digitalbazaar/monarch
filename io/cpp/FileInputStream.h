/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef FileInputStream_H
#define FileInputStream_H

#include "File.h"
#include "InputStream.h"

#include <fstream>

namespace db
{
namespace io
{

/**
 * A FileInputStream is used to read bytes from a File.
 * 
 * @author Dave Longley
 */
class FileInputStream : public db::io::InputStream
{
protected:
   /**
    * The File to read from.
    */
   File* mFile;
   
   /**
    * The ifstream to read with.
    */
   std::ifstream mStream;

public:
   /**
    * Creates a new FileInputStream that opens the passed File for reading.
    * 
    * @param file the File to read from.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   FileInputStream(File* file) throw(IOException);
   
   /**
    * Destructs this FileInputStream.
    */
   virtual ~FileInputStream();
   
   /**
    * Reads a single byte from the stream. This method will block until
    * a byte can be read or until the end of the stream is reached.
    * 
    * @param a single byte to populate from the stream.
    * 
    * @return true if a byte was read, false if the end of the stream was
    *         reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual bool read(char& b) throw(IOException);
   
   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of -1 will be returned if the end of the stream has been reached,
    * otherwise the number of bytes read will be returned.
    * 
    * @param b the array of bytes to fill.
    * @param offset the offset at which to start filling the array.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual int read(char* b, unsigned int offset, unsigned int length)
   throw(IOException);
   
   /**
    * Closes the stream.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual void close() throw(IOException);   
};

} // end namespace io
} // end namespace db
#endif
