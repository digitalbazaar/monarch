/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_zip_Zipper_H
#define db_compress_zip_Zipper_H

//#include "db/compress/deflate/Deflater.h"

namespace db
{
namespace compress
{
namespace zip
{

/**
 * A Zipper is used to compress or decompress ZIP-formatted data.
 * 
 * It can be used in conjunction with a mutator input/output stream.
 * 
 * @author Dave Longley
 */
class Zipper// :
//public virtual db::io::MutationAlgorithm,
//protected db::compress::deflate::Deflater
{
protected:
   // FIXME:
   
public:
   /**
    * Creates a new Zipper.
    */
   Zipper();
   
   /**
    * Destructs this Zipper.
    */
   virtual ~Zipper();
};

} // end namespace zip
} // end namespace compress
} // end namespace db
#endif
