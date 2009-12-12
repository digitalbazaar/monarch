/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_compress_zip_Zipper_H
#define db_compress_zip_Zipper_H

#include "monarch/compress/deflate/Deflater.h"
#include "monarch/compress/zip/ZipEntry.h"
#include "monarch/io/FileList.h"

#include <list>

namespace db
{
namespace compress
{
namespace zip
{

/**
 * A Zipper is used to compress or decompress ZIP-formatted data. There are
 * multiple ways to use a single Zipper object.
 *
 * One method is to create a Zipper object and then simply use it to
 * write out ZipEntries as they are created on the fly. Another method is
 * to create ZipEntries and store them with the Zipper, and then use its
 * hasNextEntry() and nextEntry() methods to write out the entries at
 * a later point. This method is also useful for getting a total output
 * size estimate. The last method is for simply zipping a list of files,
 * which can be done by calling zip().
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
   monarch::compress::deflate::Deflater mDeflater;

   /**
    * A ByteBuffer for processing data.
    */
   monarch::io::ByteBuffer mBuffer;

   /**
    * Typedef for a list of zip entries.
    */
   typedef std::list<ZipEntry> EntryList;

   /**
    * A list of zip entries to be written.
    */
   EntryList mUnwrittenEntries;

   /**
    * A list of the zip entries written out so far. This list is used to
    * build the central directory of the zip file.
    */
   EntryList mWrittenEntries;

   /**
    * Stores the offset to the central directory from the beginning
    * of the zip archive.
    */
   uint32_t mCentralDirectoryOffset;

   /**
    * The general purpose bit flag (this implementation always uses the
    * same one).
    */
   uint16_t mGpBitFlag;

   /**
    * The zip version and record signatures for the ZIP archive.
    */
   static const uint16_t ZIP_VERSION;
   static const uint32_t LFH_SIGNATURE;
   static const uint32_t DAD_SIGNATURE;
   static const uint32_t CDS_SIGNATURE;
   static const uint32_t CDE_SIGNATURE;

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
    * Adds a ZipEntry to be written to this Zipper. This entry can be
    * retrieved by calling nextEntry() when writing the zip archive out.
    *
    * @param ze the ZipEntry to write out.
    */
   virtual void addEntry(ZipEntry& ze);

   /**
    * Returns true if there is another ZipEntry to be written out.
    *
    * @return true if there is another ZipEntry to write, false if not.
    */
   virtual bool hasNextEntry();

   /**
    * Gets the next ZipEntry to be written out.
    *
    * @return the next ZipEntry to be written out.
    */
   virtual ZipEntry nextEntry();

   /**
    * Returns the estimated total size of the zip file, but is only accurate
    * if none of the added ZipEntries use compression, they have associated
    * input files, and none of their input files are modified before the zip
    * file is written out. This method is useful for determining archive sizes
    * before they are created.
    *
    * @return the estimated total size of the zip file.
    */
   virtual uint64_t getEstimatedArchiveSize();

   /**
    * Sets the FileList to use to produce a zip file. ZipEntries will be
    * automatically created for each file in the list and their content
    * will be compressed and written out to the passed file name.
    *
    * @param fl the FileList with files to put in the zip archive.
    * @param out the File to write the zip archive to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool zip(monarch::io::FileList& fl, monarch::io::File& out);

   /**
    * Writes a new ZipEntry.
    *
    * This will write out the local file header and store the ZipEntry
    * internally. The data for the entry must then be written via write().
    *
    * @param ze the ZipEntry to write out.
    * @param os the OutputStream to write to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeEntry(ZipEntry& ze, monarch::io::OutputStream* os);

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
   virtual int readEntry(ZipEntry& ze, monarch::io::InputStream* is);

   /**
    * Writes the passed data, for the current ZipEntry, to the passed
    * OutputStream, compressing it as it is written.
    *
    * @param b the bytes to write.
    * @param length the number of bytes in the passed buffer.
    * @param os the OutputStream to write to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(char* b, int length, monarch::io::OutputStream* os);

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
    * @return true if successful, false if an exception occurred.
    */
   virtual bool finish(monarch::io::OutputStream* os);

protected:
   /**
    * Writes out the local file header for an entry.
    *
    * @param ze the ZipEntry.
    * @param os the OutputStream to write to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeLocalFileHeader(ZipEntry& ze, monarch::io::OutputStream* os);

   /**
    * Reads an entry from a local file header.
    *
    * @param ze the ZipEntry.
    * @param is the InputStream to read from.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool readLocalFileHeader(ZipEntry& ze, monarch::io::InputStream* is);

   /**
    * Writes out the file header for an entry for the central directory.
    *
    * @param ze the ZipEntry.
    * @param os the OutputStream to write to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeFileHeader(ZipEntry& ze, monarch::io::OutputStream* os);

   /**
    * Reads an entry from a file header from the central directory.
    *
    * @param ze the ZipEntry.
    * @param is the InputStream to read from.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool readFileHeader(ZipEntry& ze, monarch::io::InputStream* is);

   /**
    * Finishes writing the current entry.
    *
    * @param os the OutputStream to write to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool finishCurrentEntry(monarch::io::OutputStream* os);
};

} // end namespace zip
} // end namespace compress
} // end namespace db
#endif
