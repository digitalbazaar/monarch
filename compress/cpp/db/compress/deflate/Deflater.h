/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_deflate_Deflater_H
#define db_compress_deflate_Deflater_H

#include "db/io/DataMutationAlgorithm.h"

#include <zlib.h>

namespace db
{
namespace compress
{
namespace deflate
{

/**
 * A Deflater is used to compress or uncompress data using the DEFLATE
 * algorithm.
 * 
 * It can be used in conjunction with a mutator input/output stream.
 * 
 * This implementation simply wraps zlib (www.zlib.org).
 * 
 * @author Dave Longley
 */
class Deflater : public db::io::DataMutationAlgorithm
{
protected:
   /**
    * The zip stream for deflating/inflating data.
    */
   z_stream mZipStream;
   
   /**
    * True when deflating, false when inflating.
    */
   bool mDeflating;
   
   /**
    * True when the current deflating/inflation has finished.
    */
   bool mFinished;
   
   /**
    * Cleans up the zip stream by deallocating any data associated with it
    * if appropriate.
    */
   virtual void cleanupStream();
   
   /**
    * Creates an Exception from a zlib return value, if necessary.
    * 
    * @param ret the zlib return value.
    * 
    * @return the exception created from the zlib return value or NULL.
    */
   virtual db::rt::Exception* createException(int ret);
   
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
    * Prepares this Deflater to deflate some data using the given parameters.
    * 
    * The level of deflation indicates the kind of compression that should be
    * used, the lower the number the less the compression but the faster the
    * algorithm with execute. The level can be 0-9 or -1 (the default, which
    * will use a compression level of 6, which is the default for zlib). A
    * level of 0 gives no compression (but adds zlib formatted data), a
    * level of 1 gives the best speed, and a level of 9 gives the best
    * compression.
    * 
    * The raw parameter indicates whether or not the zlib header and trailer
    * should be included in the output stream. If it is not included then
    * other headers and trailers, such as the gzip header/trailer, can be used
    * to wrap the raw output provided by this class.
    * 
    * Note: A Deflater cannot be used to deflate and inflate data at the same
    * time, though once the current deflation or inflation has completed, it
    * may be reused to do either operation.
    * 
    * @param level the level of compression.
    * @param raw true to output a raw DEFLATE stream, false to include a zlib
    *            header and trailer.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool startDeflating(
      int level = Z_DEFAULT_COMPRESSION,
      bool raw = true);
   
   /**
    * Prepares this Deflater to inflate some data that was previously deflated.
    * 
    * The header/trailer information will be detected automatically unless
    * raw is set to true, in which case the input data will be assumed to be
    * a raw zlib stream. Raw should be set to true when another entity is
    * processing headers and trailers for the data.
    * 
    * Note: A Deflater cannot be used to deflate and inflate data at the same
    * time, though once the current deflation or inflation has completed, it
    * may be reused to do either operation.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool startInflating(bool raw);
   
   /**
    * Updates the current deflation/inflation with more data and then
    * writes the result to the passed ByteBuffer, resizing it if appropriate
    * and if permitted.
    * 
    * @param b the bytes to deflate/inflate.
    * @param length the number of bytes to deflate/inflate.
    * @param dest the ByteBuffer to write the output to.
    * @param resize true to permit resizing the ByteBuffer, false not to.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool update(
      const char* b, int length, db::io::ByteBuffer* dest, bool resize);
   
   /**
    * Finishes the current deflation/inflation and writes any extra output
    * to the passed ByteBuffer, resizing it if appropriate and if permitted.
    * 
    * Note: If resize is false and an exception of type
    * "db.compress.deflate.InsufficientBufferSpace" with a code of 0 is
    * returned, then it is not fatal. It just means that finish should be
    * called again after emptying the buffer until all of the data can be
    * retrieved.
    * 
    * @param dest the ByteBuffer to write the output to.
    * @param resize true to permit resizing the ByteBuffer, false not to.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool finish(db::io::ByteBuffer* dest, bool resize);
   
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
   virtual int mutateData(
      db::io::ByteBuffer* src, db::io::ByteBuffer* dest, bool finish);
   
   /**
    * Gets the total number of input bytes, so far, for the current
    * deflation/inflation.
    * 
    * @return the total number of input bytes, so far, for the current
    *         deflation/inflation.
    */
   virtual unsigned long long getTotalInputBytes();
   
   /**
    * Gets the total number of output bytes, so far, for the current
    * deflation/inflation.
    * 
    * @return the total number of output bytes, so far, for the current
    *         deflation/inflation.
    */
   virtual unsigned long long getTotalOutputBytes();
};

} // end namespace deflate
} // end namespace compress
} // end namespace db
#endif
