/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_gzip_Gzipper_H
#define db_compress_gzip_Gzipper_H

#include "db/compress/deflate/Deflater.h"
#include "db/compress/gzip/Header.h"
#include "db/compress/gzip/Trailer.h"

namespace db
{
namespace compress
{
namespace gzip
{

/**
 * A Gzipper is used to compress or decompress gzip-formatted data.
 * 
 * It can be used in conjunction with a mutator input/output stream.
 * 
 * @author Dave Longley
 */
class Gzipper :
public virtual db::io::MutationAlgorithm,
protected db::compress::deflate::Deflater
{
protected:
   /**
    * The current header.
    */
   Header* mHeader;
   
   /**
    * True if the header's memory should be managed by this Gzipper, false
    * if not.
    */
   bool mCleanupHeader;
   
   /**
    * The current trailer.
    */
   Trailer* mTrailer;
   
   /**
    * True if the trailer's memory should be managed by this Gzipper, false
    * if not.
    */
   bool mCleanupTrailer;
   
   /**
    * Set to true once the header has been read/written.
    */
   bool mHeaderProcessed;
   
   /**
    * Set to true once the trailer has been read/written.
    */
   bool mTrailerProcessed;
   
   /**
    * A buffer for storing the header/trailer.
    */
   db::io::ByteBuffer mBuffer;
   
   /**
    * Used to calculate the CRC-32 as the data is processed.
    */
   unsigned int mCrc32;
   
   /**
    * True when gzip compression/decompression is complete
    * (trailer read/written).
    */
   bool mGzipFinished;
   
public:
   /**
    * Creates a new Gzipper.
    */
   Gzipper();
   
   /**
    * Destructs this Gzipper.
    */
   virtual ~Gzipper();
   
   /**
    * Sets the gzip header to use. If cleanup is true, then the passed header
    * will be deleted when this Gzipper is destructed or when this method is
    * called again with a different header.
    * 
    * Note: If this method is not called, a default header will be used.
    * 
    * @param header the gzip header to use.
    * @param cleanup true to manage the header's memory, false not to.
    */
   virtual void setHeader(Header* header, bool cleanup);
   
   /**
    * Gets the gzip header.
    * 
    * @return the gzip header.
    */
   virtual Header* getHeader();
   
   /**
    * Sets the gzip trailer to use. If cleanup is true, then the passed trailer
    * will be deleted when this Gzipper is destructed or when this method is
    * called again with a different trailer.
    * 
    * Note: If this method is not called, a default trailer will be used.
    * 
    * @param trailer the gzip trailer to use.
    * @param cleanup true to manage the trailer's memory, false not to.
    */
   virtual void setTrailer(Trailer* trailer, bool cleanup);
   
   /**
    * Gets the gzip trailer.
    * 
    * @return the gzip trailer.
    */
   virtual Trailer* getTrailer();
   
   /**
    * Prepares this Gzipper to compress some data using the given parameters.
    * 
    * The level of deflation indicates the kind of compression that should be
    * used, the lower the number the less the compression but the faster the
    * algorithm with execute. The level can be 0-9 or -1 (the default, which
    * will use a compression level of 6, which is the default for zlib). A
    * level of 0 gives no compression (but adds zlib formatted data), a
    * level of 1 gives the best speed, and a level of 9 gives the best
    * compression.
    * 
    * Note: A Gzipper cannot be used to compress and decompress data at the
    * same time, though once the current compression or decompression has
    * completed, it may be reused to do either operation.
    * 
    * @param level the level of compression.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool startCompressing(int level = Z_DEFAULT_COMPRESSION);
   
   /**
    * Prepares this Gzipper to decompress some data that was previously
    * gzip-compressed.
    * 
    * Note: A Gzipper cannot be used to compress and decompress data at the
    * same time, though once the current compression or decompression has
    * completed, it may be reused to do either operation.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool startDecompressing();
   
   /**
    * Sets the input data for the current deflation/inflation. This method
    * should be called before the initial call to process() and whenever
    * process() returns zero, if there is more input to process.
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
    * Note: The passed ByteBuffer should have at least getHeader()->getSize()
    * bytes available.
    * 
    * @param dst the ByteBuffer to write the output to.
    * @param resize true to permit resizing the ByteBuffer, false not to.
    * 
    * @return return the number of bytes written out, 0 if the buffer is
    *         empty or if there is no input, -1 if an exception occurred.
    */
   virtual int process(db::io::ByteBuffer* dst, bool resize);
   
   /**
    * Returns the amount of input data available.
    * 
    * @return the amount of input data available.
    */
   virtual unsigned int inputAvailable();
   
   /**
    * Returns whether or not this Gzipper has finished
    * compressing/decompressing.
    * 
    * @return true if this Gzipper is finished.
    */
   virtual bool isFinished();
   
   // use Deflater and mutateData()
   using Deflater::mutateData;
   
   // use Deflater getTotalInputBytes() and getTotalOutputBytes() but
   // methods refer to content, excluding gzip headers/trailers
   using Deflater::getTotalInputBytes;
   using Deflater::getTotalOutputBytes;
};

} // end namespace gzip
} // end namespace compress
} // end namespace db
#endif
