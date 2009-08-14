/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_MutationAlgorithm_H
#define db_io_MutationAlgorithm_H

#include "db/io/ByteBuffer.h"

namespace db
{
namespace io
{

/**
 * A MutationAlgorithm is used to mutate data in a source ByteBuffer and
 * write the mutated bytes to a destination ByteBuffer.
 *
 * @author Dave Longley
 */
class MutationAlgorithm
{
public:
   /**
    * An enumeration of the possible data mutation results.
    */
   enum Result
   {
      Error = -1,
      NeedsData = 0,
      Stepped = 1,
      CompleteAppend = 2,
      CompleteTruncate = 3
   };

   /**
    * Creates a new MutationAlgorithm.
    */
   MutationAlgorithm() {};

   /**
    * Destructs this MutationAlgorithm.
    */
   virtual ~MutationAlgorithm() {};

   /**
    * Gets data out of the source ByteBuffer, mutates it in some implementation
    * specific fashion, and then puts it in the destination ByteBuffer.
    *
    * The return value of this method should be:
    *
    * NeedsData: If this algorithm requires more data in the source buffer to
    * execute its next step.
    *
    * Stepped: If this algorithm had enough data to execute its next step,
    * regardless of whether or not it wrote data to the destination buffer.
    *
    * CompleteAppend: If this algorithm completed and any remaining source data
    * should be appended to the data it wrote to the destination buffer.
    *
    * CompleteTruncate: If this algorithm completed and any remaining source
    * data must be ignored (it *must not* be appended to the data written to
    * the destination buffer). The remaining source data will be untouched so
    * that it can be used for another purpose if so desired.
    *
    * Error: If an exception occurred.
    *
    * Once one a CompleteX result is returned, this method will no longer
    * be called for the same data stream.
    *
    * Note: The source and/or destination buffer may be resized by this
    * algorithm to accommodate its data needs.
    *
    * @param src the source ByteBuffer with bytes to mutate.
    * @param dst the destination ByteBuffer to write the mutated bytes to.
    * @param finish true if there will be no more source data and the mutation
    *               algorithm should finish, false if there is more data.
    *
    * @return the MutationAlgorithm::Result.
    */
   virtual Result mutateData(ByteBuffer* src, ByteBuffer* dst, bool finish) = 0;
};

} // end namespace io
} // end namespace db
#endif
