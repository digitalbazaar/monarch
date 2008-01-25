/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_gzip_Trailer_H
#define db_compress_gzip_Trailer_H

#include "db/io/ByteBuffer.h"

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
protected:
   /**
    * The CRC-32 value for this trailer.
    */
   unsigned int mCrc32;
   
   /**
    * The input size of the original data.
    */
   unsigned int mInputSize;
   
public:
   /**
    * Creates a new Trailer.
    */
   Trailer();
   
   /**
    * Destructs this Trailer.
    */
   virtual ~Trailer();
   
   /**
    * Tries to convert this header from an array of bytes.
    * 
    * @param b the array of bytes to convert from.
    * @param length the number of bytes in the array to convert from.
    * 
    * @return the number of extra bytes required to convert this header from
    *         the passed array of bytes, 0 if no more bytes are required,
    *         and -1 if an exception occurred.
    */
   virtual int convertFromBytes(char* b, int length);
   
   /**
    * Writes this header to the passed ByteBuffer, resizing it if necessary.
    * 
    * @param b the ByteBuffer to write to.
    */
   virtual void convertToBytes(db::io::ByteBuffer* b);
   
   /**
    * Sets the CRC-32 value for this trailer.
    * 
    * @param crc the CRC-32 value for this trailer.
    */
   virtual void setCrc32(unsigned int crc);
   
   /**
    * Gets the CRC-32 value of this trailer.
    * 
    * @return the CRC-32 value of this trailer.
    */
   virtual unsigned int getCrc32();
   
   /**
    * Sets the input size (ISIZE) for this trailer.
    * 
    * @param iSize the ISIZE for this trailer.
    */
   virtual void setInputSize(unsigned int iSize);
   
   /**
    * Gets the ISIZE of this trailer.
    *
    * @return the ISIZE of this trailer.
    */
   virtual unsigned int getInputSize();
};

} // end namespace gzip
} // end namespace compress
} // end namespace db
#endif
