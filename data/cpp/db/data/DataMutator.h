/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataMutator_H
#define db_data_DataMutator_H

#include "db/io/ByteBuffer.h"
#include "db/data/DataMutationAlgorithm.h"

namespace db
{
namespace data
{

/**
 * A DataMutator mutates data that is stored in a ByteBuffer and writes it to
 * another ByteBuffer from which it can be retrieved.
 * 
 * @author Dave Longley
 */
class DataMutator : protected DataMutationAlgorithm
{
protected:
   /**
    * The source ByteBuffer with unmutated data.
    */
   db::io::ByteBuffer* mSource;
   
   /**
    * The destination ByteBuffer with mutated data.
    */
   db::io::ByteBuffer* mDestination;
   
   /**
    * The algorithm used to mutate data.
    */
   DataMutationAlgorithm* mAlgorithm;
   
   /**
    * True if the data mutation algorithm has been called with the finish
    * flag set, false if not.
    */
   bool mAlgorithmFinished;
   
   /**
    * Gets data out of the source ByteBuffer, mutates it in some implementation
    * specific fashion, and then puts it in the destination ByteBuffer. The
    * actual number of mutated bytes is returned, which may be zero if there
    * are not enough bytes in the source buffer to produce mutated bytes.
    * 
    * Note: The destination buffer will be resized to accommodate any mutated
    * bytes.
    * 
    * @param src the source ByteBuffer with bytes to mutate.
    * @param dest the destination ByteBuffer to write the mutated bytes to.
    * @param finish true to finish the mutation algorithm, false not to.
    * 
    * @return true if there was enough data in the source buffer to run the
    *         mutation algorithm (which may or may not produce mutated bytes).
    */
   virtual bool mutateData(
      db::io::ByteBuffer* src, db::io::ByteBuffer* dest, bool finish);
   
public:
   /**
    * Creates a new DataMutator.
    * 
    * @param src the source ByteBuffer to read from.
    * @param dest the destination ByteBuffer to write to.
    */
   DataMutator(db::io::ByteBuffer* src, db::io::ByteBuffer* dest);
   
   /**
    * Destructs this DataMutator.
    */
   virtual ~DataMutator();
   
   /**
    * Sets the data mutation algorithm for this mutator.
    * 
    * @param algorithm the DataMutationAlgorithm to use.
    */
   virtual void setAlgorithm(DataMutationAlgorithm* algorithm);
   
   /**
    * Runs a mutation algorithm on data read from the passed input stream
    * until some mutated bytes are available.
    * 
    * This method will block until this DataMutator has mutated data that can
    * be retrieved or until the end of the input stream has been reached and
    * there is no more mutated data available.
    * 
    * @param is the input stream to read from.
    * 
    * @return 1 if mutated data is available, 0 if the end of the
    *         stream has been reached and no more mutated data is available,
    *         -1 if an IO exception occurred when reading from the stream.
    */
   virtual int mutate(db::io::InputStream* is);
   
   /**
    * Runs a mutation algorithm on data read from the passed input stream
    * and skips the mutated bytes that are generated.
    * 
    * This method will block until this DataMutator has skipped the some
    * mutated data (it will try to skip the requested amount) or until the
    * end of the input stream has been reached and there is no more mutated
    * data to skip.
    * 
    * @param is the input stream to read from.
    * @param count the number of mutated bytes to skip. 
    * 
    * @return the number of skipped mutated bytes.
    */
   virtual long long skipMutatedBytes(db::io::InputStream* is, long long count);
   
   /**
    * Gets data out of this mutator and puts it into the passed buffer. The
    * amount of data may be less than the requested amount if this mutator
    * does not have enough data.
    * 
    * @param b the buffer to put the retrieved data into.
    * @param length the maximum number of bytes to get.
    * 
    * @return the actual number of bytes retrieved, which may be 0 if this
    *         mutator is empty.
    */
   virtual int get(char* b, int length);
   
   /**
    * Returns true if this mutator has data that can be obtained, false if
    * not.
    * 
    * @return true if this mutator has data that can be obtained, false if
    *         not.
    */
   virtual bool hasData();
};

} // end namespace data
} // end namespace db
#endif
