/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataMutationAlgorithm_H
#define db_data_DataMutationAlgorithm_H

#include "db/io/ByteBuffer.h"

namespace db
{
namespace data
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
    * @return true if there was enough data in the source buffer to run the
    *         mutation algorithm (which may or may not produce mutated bytes),
    *         false if more data is required.
    */
   virtual bool mutateData(
      db::io::ByteBuffer* src, db::io::ByteBuffer* dest, bool finish) = 0;
};

} // end namespace data
} // end namespace db
#endif
