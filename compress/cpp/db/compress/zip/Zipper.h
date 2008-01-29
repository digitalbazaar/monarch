/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_compress_zip_Zipper_H
#define db_compress_zip_Zipper_H

#include "db/compress/deflate/Deflater.h"
#include "db/compress/zip/ZipEntry.h"
#include "db/io/FileList.h"

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
 * Short excerpt from APPNOTE.TXT - .ZIP File Format Specification
 * Version: 6.3.2
 * (PKWARE, Inc.)
 * 
 * Overall .ZIP file format:
 *
 *  [local file header 1]
 *  [file data 1]
 *  [data descriptor 1]
 *  . 
 *  .
 *  .
 *  [local file header n]
 *  [file data n]
 *  [data descriptor n]
 *  [archive decryption header] 
 *  [archive extra data record] 
 *  [central directory]
 *  [zip64 end of central directory record]
 *  [zip64 end of central directory locator] 
 *  [end of central directory record]
 * 
 * A. Local file header:
 *  
 *  LOCSIG = 'P'|('K'<<8)|(3<<16)|(4<<24)
 *  local file header signature     4 bytes  (0x04034b50)
 *  version needed to extract       2 bytes
 *  general purpose bit flag        2 bytes
 *  compression method              2 bytes
 *  last mod file time              2 bytes
 *  last mod file date              2 bytes
 *  crc-32                          4 bytes
 *  compressed size                 4 bytes
 *  uncompressed size               4 bytes
 *  file name length                2 bytes
 *  extra field length              2 bytes
 *  
 *  file name (variable size)
 *  extra field (variable size)
 * 
 * B. File data
 *  
 *  The compressed/stored data for the file itself.
 * 
 * C. Data descriptor (only present if bit 3 of general purpose flag is set):
 * 
 *  EXTSIG = 'P'|('K'<<8)|(7<<16)|(8<<24)
 *  data descriptor signature       4 bytes (not always present)
 *  crc-32                          4 bytes
 *  compressed size                 4 bytes
 *  uncompressed size               4 bytes
 * 
 *  Note: ZIP64 is used to handle values > 0xFFFFFFFF but
 *  this implementation doesn't support ZIP64. Therefore,
 *  only files of sizes <= 4GB are supported.
 * 
 *  The signature (0x08074b50) should be used, but isn't
 *  always present, so a parser can't assume it is.
 * 
 * D. Archive decryption header:
 *  
 *  Not supported in this implementation.
 * 
 * E. Archive extra data record:
 * 
 *  Not supported in this implementation.
 * 
 * F. Central directory structure:
 *  
 *  [file header 1]
 *  .
 *  .
 *  .
 *  [file header n]
 *  [digital signature]
 *  
 *  File header:
 *  
 *  CENSIG = 'P'|('K'<<8)|(1<<16)|(2<<24)
 *  central file header signature   4 bytes  (0x02014b50)
 *  version made by                 2 bytes
 *  version needed to extract       2 bytes
 *  general purpose bit flag        2 bytes
 *  compression method              2 bytes
 *  last mod file time              2 bytes
 *  last mod file date              2 bytes
 *  crc-32                          4 bytes
 *  compressed size                 4 bytes
 *  uncompressed size               4 bytes
 *  file name length                2 bytes
 *  extra field length              2 bytes
 *  file comment length             2 bytes
 *  disk number start               2 bytes
 *  internal file attributes        2 bytes
 *  external file attributes        4 bytes
 *  relative offset of local header 4 bytes
 *  
 *  file name (variable size)
 *  extra field (variable size)
 *  file comment (variable size)
 *  
 *  Digital signature:
 *  
 *  Not supported in this implementation.
 * 
 * G. Zip64 end of central directory record
 * 
 *  Not supported in this implementation.
 * 
 * H. Zip64 end of central directory locator
 * 
 *  Not supported in this implementation.
 * 
 * I. End of central directory record
 * 
 *  ENDSIG = 'P'|('K'<<8)|(5<<16)|(6<<24)
 *  end of central dir signature    4 bytes  (0x06054b50)
 *  number of this disk             2 bytes
 *  number of the disk with the
 *  start of the central directory  2 bytes
 *  total number of entries in the
 *  central directory on this disk  2 bytes
 *  total number of entries in
 *  the central directory           2 bytes
 *  size of the central directory   4 bytes
 *  offset of start of central
 *  directory with respect to
 *  the starting disk number        4 bytes
 *  .ZIP file comment length        2 bytes
 *  .ZIP file comment       (variable size)
 * 
 * @author Dave Longley
 */
class Zipper
{
protected:
   /**
    * The Deflater used to handle compression.
    */
   db::compress::deflate::Deflater mDeflater;
   
   /**
    * A ByteBuffer for processing data.
    */
   db::io::ByteBuffer mBuffer;
   
   /**
    * A list of the zip entries written out so far. This list is used to
    * build the central directory of the zip file.
    */
   typedef std::list<ZipEntry> EntryList;
   EntryList mEntries;
   
   /**
    * Stores the offset to the central directory from the beginning
    * of the zip archive.
    */
   unsigned int mCentralDirectoryOffset;
   
   /**
    * The general purpose bit flag (this implementation always uses the
    * same one).
    */
   unsigned short mGpBitFlag;
   
   /**
    * The zip version, compression type, and record signatures for the ZIP
    * archive.
    */
   static const unsigned short ZIP_VERSION;
   static const unsigned short COMPRESSION_METHOD;
   static const unsigned int LFH_SIGNATURE;
   static const unsigned int DAD_SIGNATURE;
   static const unsigned int CDS_SIGNATURE;
   static const unsigned int CDE_SIGNATURE;
   
   /**
    * Writes out the local file header for an entry.
    * 
    * @param ze the ZipEntry.
    * @param os the OutputStream to write to.
    * 
    * @return an exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* writeLocalFileHeader(
      ZipEntry& ze, db::io::OutputStream* os);
   
   /**
    * Reads an entry from a local file header.
    * 
    * @param ze the ZipEntry.
    * @param is the InputStream to read from.
    * 
    * @return an exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* readLocalFileHeader(
      ZipEntry& ze, db::io::InputStream* is);
   
   /**
    * Writes out the file header for an entry for the central directory.
    * 
    * @param ze the ZipEntry.
    * @param os the OutputStream to write to.
    * 
    * @return an exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* writeFileHeader(
      ZipEntry& ze, db::io::OutputStream* os);
   
   /**
    * Reads an entry from a file header from the central directory.
    * 
    * @param ze the ZipEntry.
    * @param is the InputStream to read from.
    * 
    * @return an exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* readFileHeader(
      ZipEntry& ze, db::io::InputStream* is);
   
   /**
    * Finishes writing the current entry.
    * 
    * @param os the OutputStream to write to.
    * 
    * @return an exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* finishCurrentEntry(db::io::OutputStream* os);
   
public:
   /**
    * Creates a new Zipper.
    */
   Zipper();
   
   /**
    * Destructs this Zipper.
    */
   virtual ~Zipper();
   
   /**
    * Sets the FileList to use to produce a zip file. ZipEntries will be
    * automatically created for each file in the list and their content
    * will be compressed and written out to the passed file name.
    * 
    * @param fl the FileList with files to put in the zip archive.
    * @param out the File to write the zip archive to.
    * 
    * @return an exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* zip(db::io::FileList* fl, db::io::File* out);
   
   /**
    * Writes a new ZipEntry.
    * 
    * This will write out the local file header and store the ZipEntry
    * internally. The data for the entry must then be written via write().
    * 
    * @param ze the ZipEntry to write out.
    * @param os the OutputStream to write to.
    * 
    * @return an exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* writeEntry(
      ZipEntry& ze, db::io::OutputStream* os);
   
   /**
    * Reads the next ZipEntry from the given InputStream and then orients
    * the stream for reading the entry's data.
    * 
    * @param ze the ZipEntry to populate.
    * @param is the InputStream to read from.
    * 
    * @return 1 if an entry was read, 0 if there are no more entries, or -1 if
    *         an exception occurred.
    */
   virtual int readEntry(ZipEntry& ze, db::io::InputStream* is);
   
   /**
    * Writes the passed data, for the current ZipEntry, to the passed
    * OutputStream, compressing it as it is written.
    * 
    * @param b the bytes to write.
    * @param length the number of bytes in the passed buffer.
    * @param os the OutputStream to write to.
    *
    * @return an exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* write(
      char* b, int length, db::io::OutputStream* os);
   
   /**
    * Reads some data from the current ZipEntry into the passed array of
    * bytes.
    * 
    * @param b the byte array to populate.
    * @param length the length of the passed buffer.
    *
    * @return the number of bytes written, 0 if there are no more bytes
    *         to read for the entry, and -1 if an exception occurred.
    */
   virtual int read(char* b, int length);
   
   /**
    * Finishes writing out a zip archive after all of its ZipEntries and
    * their data has been written out.
    * 
    * This method will write out the zip archive's central directory.
    * 
    * @param os the OutputStream to write to.
    * 
    * @return an exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* finish(db::io::OutputStream* os);
};

} // end namespace zip
} // end namespace compress
} // end namespace db
#endif
