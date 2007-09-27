/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_MutatorInputStream_H
#define db_data_MutatorInputStream_H

#include "db/io/FilterInputStream.h"
#include "db/io/ByteBuffer.h"
#include "db/data/DataMutator.h"

namespace db
{
namespace data
{

/**
 * A MutatorInputStream uses a DataMutator with a specific
 * DataMutationAlgorithm to mutate data as it is read.
 * 
 * @author Dave Longley
 */
class MutatorInputStream : public db::io::FilterInputStream
{
protected:
   /**
    * An internal buffer for storing data read from the underlying stream.
    */
   db::io::ByteBuffer mReadBuffer;
   
   /**
    * An internal buffer for storing mutated data.
    */
   db::io::ByteBuffer mMutatedData;
   
   /**
    * The DataMutator for this stream.
    */
   DataMutator mMutator;
   
public:
   /**
    * Creates a new MutatorInputStream that mutates data with the passed
    * DataMutationAlgorithm.
    * 
    * @param is the underlying InputStream to read from.
    * @param algorithm the DataMutationAlgorithm to use.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *                false not to.
    */
   MutatorInputStream(
      db::io::InputStream* is,
      DataMutationAlgorithm* algorithm, bool cleanup = false);
   
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
    * Skips some bytes in the stream. This method will block until the
    * some number of bytes up to specified number of bytes have been skipped
    * or the end of the stream is reached. This method will return the
    * number of bytes skipped or 0 if the end of the stream was reached or
    * -1 if an IO exception occurred.
    * 
    * @param count the number of bytes to skip.
    * 
    * @return the actual number of bytes skipped, or -1 if the end of the
    *         stream is reached or -1 if an IO exception occurred.
    */
   virtual long skip(long count);
};

} // end namespace data
} // end namespace db
#endif
