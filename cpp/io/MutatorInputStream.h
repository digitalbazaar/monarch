/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_io_MutatorInputStream_H
#define monarch_io_MutatorInputStream_H

#include "monarch/io/FilterInputStream.h"
#include "monarch/io/MutationAlgorithm.h"

namespace monarch
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
   ByteBuffer* mSource;

   /**
    * An internal buffer for storing mutated data.
    */
   ByteBuffer* mDestination;

   /**
    * True to clean up the source buffer, false not to.
    */
   bool mCleanupSource;

   /**
    * True to clean up the destination buffer, false not to.
    */
   bool mCleanupDestination;

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
    * @param src a source buffer to use internally, NULL to create one.
    * @param dst a destination buffer to use internally, NULL to create one.
    */
   MutatorInputStream(
      InputStream* is, bool cleanupStream,
      MutationAlgorithm* algorithm, bool cleanupAlgorithm,
      ByteBuffer* src = NULL, ByteBuffer* dst = NULL);

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

   /**
    * Sets the MutationAlgorithm associated with this stream.
    *
    * @param ma the MutationAlgorithm to associate with this stream.
    * @param cleanup true to clean up the algorithm when destructing,
    *                false not to.
    */
   virtual void setAlgorithm(MutationAlgorithm* ma, bool cleanup);

   /**
    * Gets the MutationAlgorithm associated with this stream.
    *
    * @return the MutationAlgorithm associated with this stream.
    */
   virtual MutationAlgorithm* getAlgorithm();
};

} // end namespace io
} // end namespace monarch
#endif
