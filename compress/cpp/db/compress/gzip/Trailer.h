/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_gzip_Trailer_H
#define db_compress_gzip_Trailer_H

namespace db
{
namespace compress
{
namespace gzip
{

/**
 * A Trailer is a trailer for gzip-formatted data. This is the trailer for
 * one gzip "member".
 * 
 * It has the following format:
 * 
 * | bytes 0-3 | bytes 4-7 | 
 * |   CRC32   |   ISIZE   |
 * 
 * CRC32 (CRC-32) = The Cyclic Redundancy Check value for the
 *                  uncompressed data.
 * ISIZE (Input SIZE) = The original size of the uncompressed data modulo 2^32.
 *  
 * Gzip format information from: http://www.gzip.org/zlib/rfc-gzip.html 
 * 
 * @author Dave Longley
 */
class Trailer
{
public:
   /**
    * Creates a new Trailer.
    */
   Trailer();
   
   /**
    * Destructs this Trailer.
    */
   virtual ~Trailer();
};

} // end namespace gzip
} // end namespace compress
} // end namespace db
#endif
