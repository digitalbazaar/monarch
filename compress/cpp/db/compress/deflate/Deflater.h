/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_deflate_Deflater_H
#define db_compress_deflate_Deflater_H

#include "db/io/DataMutationAlgorithm.h"

namespace db
{
namespace compress
{
namespace deflate
{

/**
 * An Deflater is used to compress data using the DEFLATE algorithm.
 * 
 * It should be used in conjunction with a mutator input/output stream.
 * 
 * This implementation simply wraps zlib (www.zlib.org).
 * 
 * @author Dave Longley
 */
class Deflater : public db::io::DataMutationAlgorithm
{
public:
   /**
    * Creates a new Deflater.
    */
   Deflater();
   
   /**
    * Destructs this Deflater.
    */
   virtual ~Deflater();
   
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
   virtual int mutateData(ByteBuffer* src, ByteBuffer* dest, bool finish);
};

} // end namespace deflate
} // end namespace compress
} // end namespace db
#endif
