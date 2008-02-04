/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_OutputStream_H
#define db_io_OutputStream_H

#include "db/rt/Object.h"
#include "db/rt/Collectable.h"
#include "db/io/IOException.h"

namespace db
{
namespace io
{

/**
 * An OutputStream is the abstract base class for all classes that represent an
 * output stream of bytes.
 * 
 * If an exception occurs during an operation it can be retrieved via
 * getException().
 * 
 * @author Dave Longley
 */
class OutputStream : public virtual db::rt::Object
{
public:
   /**
    * Creates a new OutputStream.
    */
   OutputStream() {};
   
   /**
    * Destructs this OutputStream.
    */
   virtual ~OutputStream() {};
   
   /**
    * Writes some bytes to the stream.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred. 
    */
   virtual bool write(const char* b, int length) = 0;
   
   /**
    * Forces this stream to flush its output, if any of it was buffered.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred. 
    */
   virtual bool flush() { return true; };
   
   /**
    * Closes the stream.
    */
   virtual void close() {};
};

// typedef for a counted reference to an OutputStream
typedef db::rt::Collectable<OutputStream> OutputStreamRef;

} // end namespace io
} // end namespace db
#endif
