/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_deflate_Inflater_H
#define db_compress_deflate_Inflater_H

#include "db/io/MutationAlgorithm.h"

#include <zlib.h>

namespace db
{
namespace compress
{
namespace deflate
{

/**
 * An Inflater is used to uncompress data that has been compressed using
 * the DEFLATE algorithm.
 * 
 * It should be used in conjunction with a mutator input/output stream.
 * 
 * This implementation simply wraps zlib (www.zlib.org).
 * 
 * @author Dave Longley
 */
class Inflater : public db::io::MutationAlgorithm
{
protected:
   /**
    * The zip stream for decompressing data.
    */
   
   
public:
   /**
    * Creates a new Inflater.
    */
   Inflater();
   
   /**
    * Destructs this Inflater.
    */
   virtual ~Inflater();
   
   virtual MutationAlgorithm::Result mutateData(
      db::io::ByteBuffer* src, db::io::ByteBuffer* dst, bool finish);
};

} // end namespace deflate
} // end namespace compress
} // end namespace db
#endif
