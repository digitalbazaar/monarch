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
    * specific fashion, and then puts it in the destination ByteBuffer.
    * 
    * If the mutation algorithm requires more data in the source buffer to
    * execute its next step then this method must return 0.
    * 
    * If the mutation algorithm had enough data to execute its next step,
    * regardless of whether or not it wrote data to the destination buffer,
    * this method must return 1.
    * 
    * If the mutation algorithm completed and any remaining source data must
    * be passed on to the destination buffer, this method must return 2.
    * 
    * If the mutation algorithm completed and any remaining source data must
    * be cleared (and *not* passed on to the destination buffer), this method
    * must return 3.
    * 
    * If an exception occurs, this method must return -1.
    * 
    * Note: The destination buffer may be resized to accommodate any mutated
    * bytes.
    * 
    * @param src the source ByteBuffer with bytes to mutate.
    * @param dest the destination ByteBuffer to write the mutated bytes to.
    * @param finish true if there will be no more source data and the
    *               mutation algorithm should finish, false if there is
    *               more data.
    * 
    * @return 0 if more data is needed, 1 if there was enough data, 2 if 
    *         the algorithm is complete and any remaining source data should
    *         be passed on, 3 if the algorithm is complete and any remaining
    *         source data should be truncated, -1 if an exception occurred.
    */
   virtual int mutateData(ByteBuffer* src, ByteBuffer* dest, bool finish) = 0;
};

} // end namespace io
} // end namespace db
#endif
