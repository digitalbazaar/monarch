/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_MutatorInputStream_H
#define db_io_MutatorInputStream_H

#include "db/io/FilterInputStream.h"
#include "db/io/MutationAlgorithm.h"

namespace db
{
namespace io
{

/**
 * A MutatorInputStream uses a MutationAlgorithm to mutate data as it is
 * read from an underlying InputStream.
 * 
 * @author Dave Longley
 */
class MutatorInputStream : public FilterInputStream
{
protected:
   /**
    * An internal buffer for storing data read from the underlying stream.
    */
   ByteBuffer mSource;
   
   /**
    * An internal buffer for storing mutated data.
    */
   ByteBuffer mDestination;
   
   /**
    * The algorithm used to mutate data.
    */
   MutationAlgorithm* mAlgorithm;
   
   /**
    * True to clean up the algorithm when destructing, false not to.
    */
   bool mCleanupAlgorithm;
   
   /**
    * Stores the last mutation result.
    */
   MutationAlgorithm::Result mResult;
   
   /**
    * Set to true once the underlying input stream has run out of data to read.
    */
   bool mSourceEmpty;
   
public:
   /**
    * Creates a new MutatorInputStream that mutates data with the passed
    * MutationAlgorithm.
    * 
    * @param is the underlying InputStream to read from.
    * @param cleanupStream true to clean up the passed InputStream when
    *                      destructing, false not to.
    * @param algorithm the MutationAlgorithm to use.
    * @param cleanupAlgorithm true to clean up the passed MutationAlgorithm
    *                         when destructing, false not to.
    */
   MutatorInputStream(
      InputStream* is, bool cleanupStream,
      MutationAlgorithm* algorithm, bool cleanupAlgorithm);
   
   /**
    * Destructs this MutatorInputStream.
    */
   virtual ~MutatorInputStream();
   
   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of 0 will be returned if the end of the stream has been reached,
    * a value of -1 will be returned if an IO exception occurred, otherwise
    * the number of bytes read will be returned.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int read(char* b, int length);
};

} // end namespace io
} // end namespace db
#endif
