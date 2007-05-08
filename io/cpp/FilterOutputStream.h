/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef FilterOutputStream_H
#define FilterOutputStream_H

#include "OutputStream.h"

namespace db
{
namespace io
{

/**
 * A FilterOutputStream is a base class for output streams that filter data
 * in some fashion before it is written out to another output stream.
 * 
 * @author Dave Longley
 */
class FilterOutputStream : public OutputStream
{
protected:
   /**
    * The underlying output stream to write to.
    */
   OutputStream* mOutputStream;
   
   /**
    * True to cleanup the underlying output stream when destructing,
    * false not to.
    */
   bool mCleanupOutputStream;
   
public:
   /**
    * Creates a new FilterOutputStream that writes to the passed OutputStream.
    * 
    * @param os the OutputStream to write to.
    * @param cleanup true to clean up the passed OutputStream when destructing,
    *                false not to.
    */
   FilterOutputStream(OutputStream* os, bool cleanup = false);
   
   /**
    * Destructs this FilterOutputStream.
    */
   virtual ~FilterOutputStream();
   
   /**
    * Writes a single byte to the stream.
    * 
    * @param b the byte to write.
    * 
    * @exception IOException thrown if an IO error occurs. 
    */
   virtual void write(const char& b) throw(IOException);
   
   /**
    * Writes some bytes to the stream.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * 
    * @exception IOException thrown if an IO error occurs. 
    */
   virtual void write(const char* b, unsigned int length) throw(IOException);
   
   /**
    * Closes the stream.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual void close();
};

} // end namespace io
} // end namespace db
#endif
