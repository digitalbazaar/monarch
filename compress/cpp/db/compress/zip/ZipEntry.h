/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_compress_zip_ZipEntry_H
#define db_compress_zip_ZipEntry_H

#include "db/rt/Collectable.h"
#include "db/io/InputStream.h"
#include "db/io/OutputStream.h"
#include "db/util/Date.h"

namespace db
{
namespace compress
{
namespace zip
{

/**
 * A ZipEntryImpl provides the implementation for a reference-counted ZipEntry.
 * 
 * A ZipEntry is typedef'ed below.
 * 
 * @author Dave Longley
 */
class ZipEntryImpl
{
protected:
   /**
    * The file name for this entry.
    */
   char* mFilename;
   
   /**
    * The file comment for this entry.
    */
   char *mFileComment;
   
   /**
    * The last modification date & time for this entry using
    * the MS-DOS date & time format.
    */
   uint32_t mDosTime;
   
   /**
    * The compressed size for this entry's data.
    */
   uint32_t mCompressedSize;
   
   /**
    * The uncompressed size for this entry's data.
    */
   uint32_t mUncompressedSize;
   
   /**
    * The crc-32 for this entry.
    */
   uint32_t mCrc32;
   
   /**
    * Stores the offset to the local header, relative to the start of the
    * first disk on which the file appears.
    */
   uint32_t mLocalHeaderOffset;
   
   /**
    * The compression method to use.
    */
   uint16_t mCompressionMethod;
   
public:
   /**
    * Creates a new ZipEntryImpl.
    */
   ZipEntryImpl();
   
   /**
    * Destructs this ZipEntryImpl.
    */
   virtual ~ZipEntryImpl();
   
   /**
    * Gets the size, in bytes, of a local file header for this entry.
    * 
    * @return the size, in bytes, of a local file header for this entry.
    */
   virtual uint32_t getLocalFileHeaderSize();
   
   /**
    * Gets the size, in bytes, of a file header for this entry.
    * 
    * @return the size, in bytes, of a file header for this entry.
    */
   virtual uint32_t getFileHeaderSize();
   
   /**
    * Sets the filename for this entry.
    * 
    * @param filename the filename for this entry.
    */
   virtual void setFilename(const char* filename);
   
   /**
    * Gets the filename for this entry.
    * 
    * @return the filename for this entry.
    */
   virtual const char* getFilename();
   
   /**
    * Sets the file comment for this entry.
    * 
    * @param comment the file comment for this entry.
    */
   virtual void setFileComment(const char* comment);
   
   /**
    * Gets the file comment for this entry.
    * 
    * @return the file comment for this entry.
    */
   virtual const char* getFileComment();
   
   /**
    * Sets the last modification date for the file.
    * 
    * @param date the last modification date for the file.
    */
   virtual void setDate(db::util::Date* date);
   
   /**
    * Sets the last modification date for the file according to a
    * MS-DOS date & time.
    * 
    * @param dosTime the MS-DOS date & time to use.
    */
   virtual void setDosTime(uint32_t dosTime);
   
   /**
    * Gets the last modification date for the file according to a
    * MS-DOS date & time.
    * 
    * @return the MS-DOS date & time to use.
    */
   virtual uint32_t getDosTime();
   
   /**
    * Sets the compressed size for this entry.
    * 
    * @param size the compressed size for this entry.
    */
   virtual void setCompressedSize(uint32_t size);
   
   /**
    * Gets the compressed size for this entry.
    * 
    * @return the compressed size for this entry.
    */
   virtual uint32_t getCompressedSize();
   
   /**
    * Sets the uncompressed size for this entry.
    * 
    * @param size the uncompressed size for this entry.
    */
   virtual void setUncompressedSize(uint32_t size);
   
   /**
    * Gets the uncompressed size for this entry.
    * 
    * @return the uncompressed size for this entry.
    */
   virtual uint32_t getUncompressedSize();
   
   /**
    * Sets the crc-32 for this entry.
    * 
    * @param crc the crc-32 for this entry.
    */
   virtual void setCrc32(uint32_t crc);
   
   /**
    * Gets the crc-32 for this entry.
    * 
    * @return the crc-32 for this entry.
    */
   virtual uint32_t getCrc32();
   
   /**
    * Sets the offset to the local file header.
    * 
    * @param offset the offset to the local file header.
    */
   virtual void setLocalFileHeaderOffset(uint32_t offset);
   
   /**
    * Gets the offset to the local file header.
    * 
    * @return the offset to the local file header.
    */
   virtual uint32_t getLocalFileHeaderOffset();
   
   /**
    * Sets whether or not compression (DEFLATE) will be used. Compression
    * is on by default. This must be called before writing the zip entry
    * or adding it to a Zipper for it to be effective.
    * 
    * @param disable true to disable compression (files will simply be archived,
    *                false to enable compression.
    */
   virtual void disableCompression(bool disable);
   
   /**
    * Gets the compression method to use with this entry.
    * 
    * @return the compression method to use.
    */
   virtual uint16_t getCompressionMethod();
};

/**
 * A ZipEntry contains information about a single file in a ZIP archive.
 * 
 * @author Dave Longley
 */
class ZipEntry : public db::rt::Collectable<ZipEntryImpl>
{
public:
   /**
    * The available compression methods.
    */
   static const uint16_t COMPRESSION_NONE;
   static const uint16_t COMPRESSION_DEFLATE;
   
   /**
    * Creates a new ZipEntry.
    */
   ZipEntry();
   
   /**
    * Creates a new ZipEntry that uses the passed ZipEntryImpl.
    * 
    * @param impl the ZipEntryImpl to use.
    */
   ZipEntry(ZipEntryImpl* impl);
   
   /**
    * Destructs this Operation.
    */
   virtual ~ZipEntry();
};

} // end namespace zip
} // end namespace compress
} // end namespace db
#endif
