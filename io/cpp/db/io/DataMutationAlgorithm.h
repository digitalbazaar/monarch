/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_DataMutationAlgorithm_H
#define db_io_DataMutationAlgorithm_H

#include "db/io/ByteBuffer.h"

namespace db
{
namespace io
{

/**
 * A DataMutationAlgorithm is used to mutate data in a source ByteBuffer and
 * write the mutated bytes to a destination ByteBuffer.
 * 
 * @author Dave Longley
 */
class DataMutationAlgorithm
{
public:
   /**
    * Creates a new DataMutationAlgorithm.
    */
   DataMutationAlgorithm() {};
   
   /**
    * Destructs this DataMutationAlgorithm.
    */
   virtual ~DataMutationAlgorithm() {};
   
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
    * @return 1 if there was enough data in the source buffer to run the
    *         mutation algorithm (which may or may not produce mutated bytes),
    *         0 if more data is required, or -1 if an exception occurred.
    */
   virtual int mutateData(ByteBuffer* src, ByteBuffer* dest, bool finish) = 0;
};

} // end namespace io
} // end namespace db
#endif
