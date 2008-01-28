/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_zip_Zipper_H
#define db_compress_zip_Zipper_H

#include "db/compress/deflate/Deflater.h"
#include "db/compress/zip/ZipEntry.h"

#include <list>

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
class Zipper : public virtual db::io::MutationAlgorithm
{
protected:
   /**
    * The Deflater used to handle compression.
    */
   db::compress::deflate::Deflater mDeflater;
   
   /**
    * A list of the zip entries written out so far. This list is used to
    * build the central directory of the zip file.
    */
   std::list<ZipEntry> mEntries;
   
public:
   /**
    * Creates a new Zipper.
    */
   Zipper();
   
   /**
    * Destructs this Zipper.
    */
   virtual ~Zipper();
   
   // FIXME: interface is going to be different from deflater/gzipper
   
   // method for parsing a FileList into ZipEntries
   // then use mutator input stream
   
   // alternatively, use interface to start zip entry
   // and then write compressed data to an outputstream
   
   // then finish() to write out central directory
   
   // FIXME: for reading, need to be able to iterate over entries
   // so read from an input stream until the next entry is read
   // this will orient the passed stream at the next entry
   
   /**
    * Starts a new ZipEntry.
    * 
    * If reading a zip file, then the passed ZipEntry will be populated with
    * the next local file header.
    * 
    * This will write out the local file header and store the ZipEntry
    * internally. The data for the entry must then be written via write().
    * 
    * @param ze the ZipEntry to start.
    * 
    * @return an exception if one occurred, NULL if not.
    */
   //virtual db::rt::Exception* startEntry(ZipEntry& ze);
   
   /**
    * Sets the input data for the current ZipEntry. This method should be
    * called before the initial call to process() and whenever process()
    * returns zero, if there is more input to process.
    * 
    * @param b the bytes to deflate/inflate.
    * @param length the number of bytes to deflate/inflate.
    * @param finish true if the passed data is the last data to process.
    */
   //virtual void setInput(const char* b, int length, bool finish);
   
   /**
    * Processes the current ZipEntry and its data (which was set via
    * startEntry() and setInput()) and writes the resulting output to the
    * passed ByteBuffer, resizing it if appropriate and if permitted.
    * 
    * @param dst the ByteBuffer to write the output to.
    * @param resize true to permit resizing the ByteBuffer, false not to.
    * 
    * @return return the number of bytes written out, 0 if the buffer is
    *         empty or if there is no input, -1 if an exception occurred.
    */
   //virtual int process(db::io::ByteBuffer* dst, bool resize);
   
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
};

} // end namespace zip
} // end namespace compress
} // end namespace db
#endif
