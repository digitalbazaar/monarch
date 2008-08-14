/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_MutatorOutputStream_H
#define db_io_MutatorOutputStream_H

#include "db/io/FilterOutputStream.h"
#include "db/io/MutationAlgorithm.h"

namespace db
{
namespace io
{

/**
 * A MutatorOutputStream uses a MutationAlgorithm to mutate data as it is
 * written to this stream.
 * 
 * @author Dave Longley
 */
class MutatorOutputStream : public FilterOutputStream
{
protected:
   /**
    * An internal buffer for storing data written to this stream that is
    * unmutated.
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
    * A ByteBuffer used as a wrapper for source bytes to improve performance
    * when input data doesn't need to be cached.
    */
   ByteBuffer mInputWrapper;
   
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
    * True if finished writing output, false if not.
    */
   bool mFinished;
   
public:
   /**
    * Creates a new MutatorOutputStream that mutates data with the passed
    * MutationAlgorithm.
    * 
    * @param os the OutputStream to write mutated data to.
    * @param cleanupStream true to clean up the passed OutputStream when
    *                      destructing, false not to.
    * @param algorithm the MutationAlgorithm to use.
    * @param cleanupAlgorithm true to clean up the passed MutationAlgorithm
    *                         when destructing, false not to.
    * @param src a source buffer to use internally, NULL to create one.
    * @param dst a destination buffer to use internally, NULL to create one. 
    */
   MutatorOutputStream(
      OutputStream* is, bool cleanupStream,
      MutationAlgorithm* algorithm, bool cleanupAlgorithm,
      ByteBuffer* src = NULL, ByteBuffer* dst = NULL);
   
   /**
    * Destructs this MutatorOutputStream.
    */
   virtual ~MutatorOutputStream();
   
   /**
    * Writes some bytes to the stream.
    * 
    * Note: Passing a length of 0 will signal the underlying mutation algorithm
    * to finish.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred. 
    */
   virtual bool write(const char* b, int length);
   
   /**
    * Forces this stream to finish its output, if the stream has such a
    * function.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool finish();
   
   /**
    * Closes the stream and ensures the mutation algorithm finished.
    */
   virtual void close();
   
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
} // end namespace db
#endif
