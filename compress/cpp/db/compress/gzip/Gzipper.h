/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_gzip_Gzipper_H
#define db_compress_gzip_Gzipper_H

#include "db/compress/deflate/Deflater.h"

namespace db
{
namespace compress
{
namespace gzip
{

/**
 * A Gzipper is used to compress or uncompress gzip-formatted data.
 * 
 * It can be used in conjunction with a mutator input/output stream.
 * 
 * @author Dave Longley
 */
class Gzipper : public db::io::MutationAlgorithm
{
protected:
   /**
    * The Deflater used to deflate/inflate data.
    */
   db::compress::deflate::Deflater mDeflater;
   
public:
   /**
    * Creates a new Gzipper.
    */
   Gzipper();
   
   /**
    * Destructs this Gzipper.
    */
   virtual ~Gzipper();
};

} // end namespace gzip
} // end namespace compress
} // end namespace db
#endif
