/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_deflate_Deflater_H
#define db_compress_deflate_Deflater_H

#include "db/io/MutationAlgorithm.h"

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
class Deflater : public db::io::MutationAlgorithm
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
    * True when the current deflation/inflation should finish.
    */
   bool mShouldFinish;
   
   /**
    * True when the current deflation/inflation has finished.
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
    * Sets the input data for the current deflation/inflation. This method
    * should be called before the initial call to update() and whenever
    * update() returns zero, if there is more input to process.
    * 
    * @param b the bytes to deflate/inflate.
    * @param length the number of bytes to deflate/inflate.
    * @param finish true if the passed data is the last data to process.
    */
   virtual void setInput(const char* b, int length, bool finish);
   
   /**
    * Processes the current input (which was set via setInput()) and writes
    * the resulting output to the passed ByteBuffer, resizing it if appropriate
    * and if permitted.
    * 
    * @param dst the ByteBuffer to write the output to.
    * @param resize true to permit resizing the ByteBuffer, false not to.
    * 
    * @return return the number of bytes written out, 0 if the buffer is
    *         empty or if there is no input, -1 if an exception occurred.
    */
   virtual int process(db::io::ByteBuffer* dst, bool resize);
   
   /**
    * Gets data out of the source ByteBuffer, mutates it in some implementation
    * specific fashion, and then puts it in the destination ByteBuffer.
    * 
    * The return value of this method should be:
    * 
    * NeedsData: If this algorithm requires more data in the source buffer to
    * execute its next step.
    * 
    * Stepped: If this algorithm had enough data to execute its next step,
    * regardless of whether or not it wrote data to the destination buffer.
    * 
    * CompleteAppend: If this algorithm completed and any remaining source data
    * should be appended to the data it wrote to the destination buffer.
    * 
    * CompleteTruncate: If this algorithm completed and any remaining source
    * data must be cleared (it *must not* be appended to the data written to
    * the destination buffer).
    * 
    * Error: If an exception occurred.
    * 
    * Once one a CompleteX result is returned, this method will no longer
    * be called for the same data stream.
    * 
    * Note: The source and/or destination buffer may be resized by this
    * algorithm to accommodate its data needs.
    * 
    * @param src the source ByteBuffer with bytes to mutate.
    * @param dst the destination ByteBuffer to write the mutated bytes to.
    * @param finish true if there will be no more source data and the mutation
    *               algorithm should finish, false if there is more data.
    * 
    * @return the MutationAlgorithm::Result.
    */
   virtual MutationAlgorithm::Result mutateData(
      db::io::ByteBuffer* src, db::io::ByteBuffer* dst, bool finish);
   
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
