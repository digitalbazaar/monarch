/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/compress/zip/Zipper.h"

#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/rt/Iterator.h"
#include "db/util/Data.h"

#include <cstring>

using namespace db::compress::zip;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

// define zip version
const uint16_t Zipper::ZIP_VERSION = 20;

// define record signatures for ZIP format
const uint32_t Zipper::LFH_SIGNATURE = 'P'|('K'<<8)|(3<<16)|(4<<24);
const uint32_t Zipper::DAD_SIGNATURE = 'P'|('K'<<8)|(7<<16)|(8<<24);
const uint32_t Zipper::CDS_SIGNATURE = 'P'|('K'<<8)|(1<<16)|(2<<24);
const uint32_t Zipper::CDE_SIGNATURE = 'P'|('K'<<8)|(5<<16)|(6<<24);

Zipper::Zipper() :
   mBuffer(0)
{
   mCentralDirectoryOffset = 0;
   
   // bit 0 = 0, no encryption
   // bit 1 = 0, normal compression used (deflate default level)
   // bit 2 = 0, normal compression used (deflate default level)
   // bit 3 = 1, local header crc & sizes = 0, data descriptors used, CD used
   // bit 4 = 0, reserved for enhanced deflating
   // bit 5 = 0, not compressed patched data
   // bit 6 = 0, no AES encryption used
   // bit 7 = 0, unused
   // bit 8 = 0, unused
   // bit 9 = 0, unused
   // bit 10 = 0, unused
   // bit 11 = 0, if set, UTF-8 must be used for filenames
   // bit 12 = 0, reserved for PKWARE enhanced compression
   // bit 13 = 0, CD (central directory) not encrypted
   // bit 14 = 0, reserved for PKWARE
   // bit 15 = 0, reserved for PKWARE
   mGpBitFlag = 0x08;
}

Zipper::~Zipper()
{
}

void Zipper::addEntry(ZipEntry& ze)
{
   mUnwrittenEntries.push_back(ze);
}

bool Zipper::hasNextEntry()
{
   return !mUnwrittenEntries.empty();
}

ZipEntry Zipper::nextEntry()
{
   ZipEntry rval(NULL);
   
   if(!mUnwrittenEntries.empty())
   {
      rval = mUnwrittenEntries.front();
      mUnwrittenEntries.pop_front();
   }
   
   return rval;
}

uint64_t Zipper::getEstimatedArchiveSize()
{
   uint64_t rval = 0;
   
   for(EntryList::iterator i = mUnwrittenEntries.begin();
       i != mUnwrittenEntries.end(); i++)
   {
      ZipEntry& ze = *i;
      
      // get filename length
      int filenameLength = strlen(ze->getFilename());
      
      // 30 bytes for each local file header + filename + extra (0)
      rval += 30 + filenameLength + 0;
      
      // add file size
      File inputFile = ze->getInputFile();
      if(!inputFile.isNull())
      {
         rval += inputFile->getLength();
      }
      
      // 16 bytes for data descriptor
      rval += 16;
      
      // 46 bytes for each file header + filename + extra (0) + file comment
      rval += 46 + filenameLength + 0 + strlen(ze->getFileComment());
      
      // 22 bytes for end of central directory record
      rval += 22;
   }
   
   return rval;
}

bool Zipper::zip(FileList& fl, File& out)
{
   bool rval = true;
   
   // clear entry list, reset central directory offset
   mWrittenEntries.clear();
   mCentralDirectoryOffset = 0;
   
   // create file output stream
   FileOutputStream fos(out);
   
   // create zip entries and write them out
   char b[2048];
   int numBytes;
   IteratorRef<File> i = fl->getIterator();
   while(rval && i->hasNext())
   {
      File& file = i->next();
      
      // create entry, set file name
      ZipEntry ze;
      ze->setFilename(file->getAbsolutePath());
      
      // write entry
      if((rval = writeEntry(ze, &fos)))
      {
         // write data for entry
         FileInputStream fis(file);
         while(rval && (numBytes = fis.read(b, 2048)) > 0)
         {
            rval = write(b, numBytes, &fos);
         }
         fis.close();
      }
   }
   
   // finish zip archive
   finish(&fos);
   
   // close file stream
   fos.close();
   
   return rval;
}

bool Zipper::writeEntry(ZipEntry& ze, OutputStream* os)
{
   // finish the current entry
   bool rval = finishCurrentEntry(os);
   
   if(rval)
   {
      // store new entry in list
      mWrittenEntries.push_back(ze);
      
      // reset entry crc
      ze->setCrc32(0);
      
      if(ze->getCompressionMethod() == ZipEntry::COMPRESSION_DEFLATE)
      {
         // start raw deflation
         mDeflater.startDeflating(-1, true);
      }
      
      // write out local file header for entry
      rval = writeLocalFileHeader(ze, os);
   }
   
   return rval;
}

int Zipper::readEntry(ZipEntry& ze, InputStream* is)
{
   int rval = -1;
   
   // FIXME: not implemented
   ExceptionRef e = new Exception("Zipper::readEntry() not implemented.");
   Exception::setLast(e, false);
   
   return rval;
}

bool Zipper::write(char* b, int length, OutputStream* os)
{
   bool rval = true;
   
   // update the entry's crc
   ZipEntry& ze = mWrittenEntries.back();
   ze->setCrc32(crc32(ze->getCrc32(), (unsigned char*)b, length));
   
   if(ze->getCompressionMethod() == ZipEntry::COMPRESSION_DEFLATE)
   {
      // initialize buffer
      if(mBuffer.capacity() == 0)
      {
         mBuffer.resize(2048);
      }
      
      // set input for deflater
      mDeflater.setInput(b, length, false);
      
      // process all input and write it to the output stream
      while(rval &&
            (mDeflater.process(&mBuffer, false) > 0 || mBuffer.isFull()))
      {
         // ensure data is written to output stream
         rval = (mBuffer.get(os) > 0);
      }
   }
   else
   {
      // write all input to the output stream
      rval = os->write(b, length);
      if(rval)
      {
         // update uncompressed/compressed bytes in entry
         uint32_t size = ze->getUncompressedSize();
         ze->setCompressedSize(size + length);
         ze->setUncompressedSize(size + length);
      }
   }
   
   return rval;
}

int Zipper::read(char* b, int length)
{
   int rval = -1;
   
   // FIXME: not implemented
   ExceptionRef e = new Exception("Zipper::read() not implemented.");
   Exception::setLast(e, false);
   
   return rval;
}

bool Zipper::finish(OutputStream* os)
{
   // finish the current entry
   bool rval = finishCurrentEntry(os);
   
   // check central directory flag
   if(rval && true)// FIXME: replace "true" with flag check
   {
      // write out the file header for each entry
      uint32_t cdSize = 0;
      for(EntryList::iterator i = mWrittenEntries.begin();
          rval && i != mWrittenEntries.end(); i++)
      {
         rval = writeFileHeader(*i, os);
         cdSize += (*i)->getFileHeaderSize();
      }
      
      // write out end of central directory record
      if(rval)
      {
         uint32_t cdesig = DB_UINT32_TO_LE(CDE_SIGNATURE);
         uint16_t diskNumber = 0;
         uint16_t entries = DB_UINT16_TO_LE(mWrittenEntries.size());
         uint16_t zipCommentLength = 0;
         uint32_t offset = DB_UINT32_TO_LE(mCentralDirectoryOffset);
         
         // write out end of central directory record signature
         // write out this disk number (0)
         // write out number of disk with the start of central directory (0)
         // write out number of entries in central directory on this disk
         // write out total number of entries in central directory
         // write out size of central directory
         // write out offset of start of central directory w/respect to disk #
         // write out .ZIP file comment length
         // write out .ZIP file comment
         rval =
            os->write((char*)&cdesig, 4) &&
            os->write((char*)&diskNumber, 2) &&
            os->write((char*)&diskNumber, 2) &&
            os->write((char*)&entries, 2) &&
            os->write((char*)&entries, 2) &&
            os->write((char*)&cdSize, 4) &&
            os->write((char*)&offset, 4) &&
            os->write((char*)&zipCommentLength, 2);
      }
   }
   
   // clear entry list, reset central directory offset
   mWrittenEntries.clear();
   mCentralDirectoryOffset = 0;
   
   return rval;
}

bool Zipper::writeLocalFileHeader(ZipEntry& ze, OutputStream* os)
{
   uint32_t lfhsig = DB_UINT32_TO_LE(LFH_SIGNATURE);
   uint16_t zipver = DB_UINT16_TO_LE(ZIP_VERSION);
   uint16_t gpflag = DB_UINT16_TO_LE(mGpBitFlag);
   uint16_t compmd = DB_UINT16_TO_LE(ze->getCompressionMethod());
   uint32_t dosTime = DB_UINT32_TO_LE(ze->getDosTime());
   uint32_t crc = 0;
   uint32_t size = 0;
   uint16_t fnLength = DB_UINT16_TO_LE(strlen(ze->getFilename()));
   uint16_t exLength = 0;
   
   // write local file header signature
   // write version needed to extract
   // write general purpose bit flag
   // write compression method
   // write last mod file time (combined into MS-DOS date & time)
   // write last mod file date (combined into MS-DOS date & time)
   // write crc-32 (0 here since its in the central directory)
   // write compressed size (0 here since its in the central directory)
   // write uncompressed size (0 here since its in the central directory)
   // write file name length
   // write extra field length (0)
   return
      os->write((char*)&lfhsig, 4) &&
      os->write((char*)&zipver, 2) &&
      os->write((char*)&gpflag, 2) &&
      os->write((char*)&compmd, 2) &&
      os->write((char*)&dosTime, 4) &&
      os->write((char*)&crc, 4) &&
      os->write((char*)&size, 4) &&
      os->write((char*)&size, 4) &&
      os->write((char*)&fnLength, 2) &&
      os->write((char*)&exLength, 2) &&
      os->write(ze->getFilename(), fnLength);
}

bool Zipper::readLocalFileHeader(ZipEntry& ze, InputStream* is)
{
   // FIXME: not implemented
   ExceptionRef e = new Exception(
      "Zipper::readLocalFileHeader() not implemented.");
   Exception::setLast(e, false);
   return false;
}

bool Zipper::writeFileHeader(ZipEntry& ze, OutputStream* os)
{
   uint32_t cdssig = DB_UINT32_TO_LE(CDS_SIGNATURE);
   uint16_t zipver = DB_UINT16_TO_LE(ZIP_VERSION);
   uint16_t gpflag = DB_UINT16_TO_LE(mGpBitFlag);
   uint16_t compmd = DB_UINT16_TO_LE(ze->getCompressionMethod());
   uint32_t dosTime = DB_UINT32_TO_LE(ze->getDosTime());
   uint32_t crc = DB_UINT32_TO_LE(ze->getCrc32());
   uint32_t cSize = DB_UINT32_TO_LE(ze->getCompressedSize());
   uint32_t ucSize = DB_UINT32_TO_LE(ze->getUncompressedSize());
   uint16_t fnLength = DB_UINT16_TO_LE(strlen(ze->getFilename()));
   uint16_t exLength = 0;
   uint16_t fcLength = DB_UINT16_TO_LE(strlen(ze->getFileComment()));
   uint16_t diskNumber = 0;
   uint16_t internalAttr = 0;
   uint32_t externalAttr = 0;
   uint32_t offset = DB_UINT32_TO_LE(ze->getLocalFileHeaderOffset());
   
   // write central file header signature
   // write version made by
   // write version needed to extract
   // write general purpose bit flag
   // write compression method
   // write last mod file time (combined into MS-DOS date & time)
   // write last mod file date (combined into MS-DOS date & time)
   // write crc-32
   // write compressed size
   // write uncompressed size
   // write file name length
   // write extra field length (0)
   // write file comment length
   // write disk number start
   // write internal file attributes
   // write external file attributes
   // write relative offset of local header
   // write file name
   // write extra field (none)
   // write file comment
   return
      os->write((char*)&cdssig, 4) &&
      os->write((char*)&zipver, 2) &&
      os->write((char*)&zipver, 2) &&
      os->write((char*)&gpflag, 2) &&
      os->write((char*)&compmd, 2) &&
      os->write((char*)&dosTime, 4) &&
      os->write((char*)&crc, 4) &&
      os->write((char*)&cSize, 4) &&
      os->write((char*)&ucSize, 4) &&
      os->write((char*)&fnLength, 2) &&
      os->write((char*)&exLength, 2) &&
      os->write((char*)&fcLength, 2) &&
      os->write((char*)&diskNumber, 2) &&
      os->write((char*)&internalAttr, 2) &&
      os->write((char*)&externalAttr, 4) &&
      os->write((char*)&offset, 4) &&
      os->write(ze->getFilename(), fnLength) &&
      // no extra field to write out, write out comment
      os->write(ze->getFileComment(), fcLength);
}

bool Zipper::readFileHeader(ZipEntry& ze, InputStream* is)
{
   // FIXME: not implemented
   ExceptionRef e = new Exception("Zipper::readFileHeader() not implemented.");
   Exception::setLast(e, false);
   return false;
}

bool Zipper::finishCurrentEntry(OutputStream* os)
{
   bool rval = true;
   
   // write data descriptor for previous entry
   if(!mWrittenEntries.empty())
   {
      // get the current entry
      ZipEntry& ze = mWrittenEntries.back();
      
      if(ze->getCompressionMethod() == ZipEntry::COMPRESSION_DEFLATE)
      {
         // write out any remaining deflated data
         mDeflater.setInput(NULL, 0, true);
         while(rval && (mDeflater.process(&mBuffer, false) > 0 ||
               mBuffer.isFull()))
         {
            // ensure data is written to output stream
            rval = (mBuffer.get(os) > 0);
         }
         
         // set compressed/uncompressed sizes for entry
         ze->setCompressedSize(mDeflater.getTotalOutputBytes());
         ze->setUncompressedSize(mDeflater.getTotalInputBytes());
      }
      
      // store offset to local file header
      ze->setLocalFileHeaderOffset(mCentralDirectoryOffset);
      
      // add size of local file header to central directory offset
      mCentralDirectoryOffset += ze->getLocalFileHeaderSize();
      
      // add size of compressed data to central directory offset
      mCentralDirectoryOffset += ze->getCompressedSize();
      
      if(rval)
      {
         uint32_t dadsig = DB_UINT32_TO_LE(DAD_SIGNATURE);
         uint32_t crc = DB_UINT32_TO_LE(ze->getCrc32());
         uint32_t cSize = DB_UINT32_TO_LE(ze->getCompressedSize());
         uint32_t ucSize = DB_UINT32_TO_LE(ze->getUncompressedSize());
         
         // write out data descriptor signature
         // write out crc
         // write out compressed size
         // write out uncompressed size
         rval =
            os->write((char*)&dadsig, 4) &&
            os->write((char*)&crc, 4) &&
            os->write((char*)&cSize, 4) &&
            os->write((char*)&ucSize, 4);
         
         // add data descriptor size to central directory offset
         mCentralDirectoryOffset += 16;
      }
   }
   
   return rval;
}
