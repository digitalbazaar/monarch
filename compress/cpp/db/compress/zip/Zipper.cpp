/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/zip/Zipper.h"

#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/util/Iterator.h"

using namespace db::compress::zip;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

// define zip version
const unsigned short Zipper::ZIP_VERSION = 20;

// define compression method (DEFLATE = 8)
const unsigned short Zipper::COMPRESSION_METHOD = 8;

// define record signatures for ZIP format
const unsigned int Zipper::LFH_SIGNATURE = 'P'|('K'<<8)|(3<<16)|(4<<24);
const unsigned int Zipper::DAD_SIGNATURE = 'P'|('K'<<8)|(7<<16)|(8<<24);
const unsigned int Zipper::CDS_SIGNATURE = 'P'|('K'<<8)|(1<<16)|(2<<24);
const unsigned int Zipper::CDE_SIGNATURE = 'P'|('K'<<8)|(5<<16)|(6<<24);

Zipper::Zipper() :
   mBuffer(2048)
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

Exception* Zipper::zip(FileList* fl, File* out)
{
   Exception* rval = NULL;
   
   // clear entry list, reset central directory offset
   mEntries.clear();
   mCentralDirectoryOffset = 0;
   
   // create file output stream
   FileOutputStream fos(out);
   
   // create zip entries and write them out
   char b[2048];
   int numBytes;
   Iterator<File*>* i = fl->getIterator();
   while(rval == NULL && i->hasNext())
   {
      File* file = i->next();
      
      // create entry, set file name
      ZipEntry ze;
      ze->setFilename(file->getName());
      
      // write entry
      rval = writeEntry(ze, &fos);
      if(rval == NULL)
      {
         // write data for entry
         FileInputStream fis(file);
         while(rval == NULL && (numBytes = fis.read(b, 2048)) > 0)
         {
            rval = write(b, numBytes, &fos);
         }
         fis.close();
      }
   }
   delete i;
   
   // finish zip archive
   finish(&fos);
   
   // close file stream
   fos.close();
   
   return rval;
}

Exception* Zipper::writeLocalFileHeader(ZipEntry& ze, OutputStream* os)
{
   Exception* rval = NULL;
   
   // FIXME: handle byte ordering (format is little-endian)
   
   unsigned int dosTime = ze->getDosTime();
   unsigned int crc = 0;
   unsigned int size = 0;
   unsigned short fnLength = strlen(ze->getFilename());
   unsigned short exLength = 0;
   
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
   if(!(
      os->write((char*)&LFH_SIGNATURE, 4) &&
      os->write((char*)&ZIP_VERSION, 2) &&
      os->write((char*)&mGpBitFlag, 2) &&
      os->write((char*)&COMPRESSION_METHOD, 2) &&
      os->write((char*)&dosTime, 4) &&
      os->write((char*)&crc, 4) &&
      os->write((char*)&size, 4) &&
      os->write((char*)&size, 4) &&
      os->write((char*)&fnLength, 2) &&
      os->write((char*)&exLength, 2) &&
      os->write(ze->getFilename(), fnLength)))
   {
      // could not write to output stream
      rval = Exception::getLast();
   }
   
   return rval;
}

Exception* Zipper::readLocalFileHeader(ZipEntry& ze, InputStream* is)
{
   Exception* rval = NULL;
   
   // FIXME: not implemented
   rval = new Exception("Zipper::readLocalFileHeader() not implemented!");
   Exception::setLast(rval);
   
   return rval;
}

Exception* Zipper::writeFileHeader(ZipEntry& ze, OutputStream* os)
{
   Exception* rval = NULL;
   
   // FIXME: handle byte ordering (format is little-endian)
   
   unsigned int dosTime = ze->getDosTime();
   unsigned int crc = ze->getCrc32();
   unsigned int cSize = ze->getCompressedSize();
   unsigned int ucSize = ze->getUncompressedSize();
   unsigned short fnLength = strlen(ze->getFilename());
   unsigned short exLength = 0;
   unsigned short fcLength = strlen(ze->getFileComment());
   unsigned short diskNumber = 0;
   unsigned short internalAttr = 0;
   unsigned int externalAttr = 0;
   unsigned int offset = ze->getLocalFileHeaderOffset();
   
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
   if(!(
      os->write((char*)&CDS_SIGNATURE, 4) &&
      os->write((char*)&ZIP_VERSION, 2) &&
      os->write((char*)&ZIP_VERSION, 2) &&
      os->write((char*)&mGpBitFlag, 2) &&
      os->write((char*)&COMPRESSION_METHOD, 2) &&
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
      os->write(ze->getFileComment(), fcLength)))
   {
      // could not write to output stream
      rval = Exception::getLast();
   }
   
   return rval;
}

Exception* Zipper::readFileHeader(ZipEntry& ze, InputStream* is)
{
   Exception* rval = NULL;
   
   // FIXME: not implemented
   rval = new Exception("Zipper::readFileHeader() not implemented!");
   Exception::setLast(rval);
   
   return rval;
}

Exception* Zipper::finishCurrentEntry(OutputStream* os)
{
   Exception* rval = NULL;
   
   // write data descriptor for previous entry
   if(!mEntries.empty())
   {
      // get the current entry
      ZipEntry& ze = mEntries.back();
      
      // write out any remaining deflated data
      mDeflater.setInput(NULL, 0, true);
      mBuffer.clear();
      mDeflater.process(&mBuffer, true);
      if(!mBuffer.isEmpty())
      {
         if(mBuffer.get(os) == 0)
         {
            // could not write to output stream
            rval = Exception::getLast();
         }
      }
      
      // set compressed/uncompressed sizes for entry
      ze->setCompressedSize(mDeflater.getTotalOutputBytes());
      ze->setUncompressedSize(mDeflater.getTotalInputBytes());
      
      // store offset to local file header
      ze->setLocalFileHeaderOffset(mCentralDirectoryOffset);
      
      // add size of local file header to central directory offset
      mCentralDirectoryOffset += ze->getLocalFileHeaderSize();
      
      // add size of compressed data to central directory offset
      mCentralDirectoryOffset += mDeflater.getTotalOutputBytes();
      
      if(rval == NULL)
      {
         unsigned int crc = ze->getCrc32();
         unsigned int cSize = ze->getCompressedSize();
         unsigned int ucSize = ze->getUncompressedSize();
         
         // FIXME: handle byte ordering (format is little-endian)
         
         // write out data descriptor signature
         // write out crc
         // write out compressed size
         // write out uncompressed size
         if(!(
            os->write((char*)&DAD_SIGNATURE, 4) &&
            os->write((char*)&crc, 4) &&
            os->write((char*)&cSize, 4) &&
            os->write((char*)&ucSize, 4)))
         {
            // could not write to output stream
            rval = Exception::getLast();
         }
         
         // add data descriptor size to central directory offset
         mCentralDirectoryOffset += 16;
      }
   }
   
   return rval;
}

Exception* Zipper::writeEntry(ZipEntry& ze, OutputStream* os)
{
   // finish the current entry
   Exception* rval = finishCurrentEntry(os);
   
   if(rval == NULL)
   {
      // store new entry in list
      mEntries.push_back(ze);
      
      // reset entry crc
      ze->setCrc32(0);
      
      // start raw deflation
      mDeflater.startDeflating(-1, true);
      
      // write out local file header for entry
      rval = writeLocalFileHeader(ze, os);
   }
   
   return rval;
}

int Zipper::readEntry(ZipEntry& ze, InputStream* is)
{
   int rval = -1;
   
   // FIXME: not implemented
   Exception::setLast(new Exception("Zipper::readEntry() not implemented!"));
   
   return rval;
}

Exception* Zipper::write(char* b, int length, OutputStream* os)
{
   Exception* rval = NULL;
   
   // update the entry's crc
   ZipEntry& ze = mEntries.back();
   ze->setCrc32(crc32(ze->getCrc32(), (unsigned char*)b, length));
   
   // set input for deflater
   mDeflater.setInput(b, length, false);
   
   // process all input and write it to the output stream
   while(rval == NULL && mDeflater.process(&mBuffer, false) > 0)
   {
      if(mBuffer.get(os) == 0)
      {
         // could not write to output stream
         rval = Exception::getLast();
      }
   }
   
   return rval;
}

int Zipper::read(char* b, int length)
{
   int rval = -1;
   
   // FIXME: not implemented
   Exception::setLast(new Exception("Zipper::read() not implemented!"));
   
   return rval;
}

Exception* Zipper::finish(OutputStream* os)
{
   // finish the current entry
   Exception* rval = finishCurrentEntry(os);
   
   // check central directory flag
   if(rval == NULL && true)// FIXME: replace "true" with flag check
   {
      // write out the file header for each entry
      unsigned int cdSize = 0;
      for(EntryList::iterator i = mEntries.begin();
          rval == NULL && i != mEntries.end(); i++)
      {
         rval = writeFileHeader(*i, os);
         cdSize += (*i)->getFileHeaderSize();
      }
      
      // write out end of central directory record
      if(rval == NULL)
      {
         // FIXME: handle byte ordering (format is little-endian)
         
         unsigned short diskNumber = 0;
         unsigned short entries = mEntries.size();
         unsigned short zipCommentLength = 0;
         
         // write out end of central directory record signature
         // write out this disk number (0)
         // write out number of disk with the start of central directory (0)
         // write out number of entries in central directory on this disk
         // write out total number of entries in central directory
         // write out size of central directory
         // write out offset of start of central directory w/respect to disk #
         // write out .ZIP file comment length
         // write out .ZIP file comment
         if(!(
            os->write((char*)&CDE_SIGNATURE, 4) &&
            os->write((char*)&diskNumber, 2) &&
            os->write((char*)&diskNumber, 2) &&
            os->write((char*)&entries, 2) &&
            os->write((char*)&entries, 2) &&
            os->write((char*)&cdSize, 4) &&
            os->write((char*)&mCentralDirectoryOffset, 4) &&
            os->write((char*)&zipCommentLength, 2)))
         {
            // could not write to output stream
            rval = Exception::getLast();
         }
      }
   }
   
   // clear entry list, reset central directory offset
   mEntries.clear();
   mCentralDirectoryOffset = 0;
   
   return rval;
}
