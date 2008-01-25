/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/gzip/Header.h"

#include "db/rt/System.h"

using namespace std;
using namespace db::compress::gzip;
using namespace db::io;
using namespace db::rt;

Header::Header()
{
   // initialize empty header
   mHasFilename = false;
   mFilename = NULL;
   mHasFileComment = false;
   mFileComment = NULL;
   mHasCrc = false;
   mCrc = 0;
}

Header::~Header()
{
   if(mFilename != NULL)
   {
      free(mFilename);
   }
   
   if(mFileComment != NULL)
   {
      free(mFileComment);
   }
}

int Header::convertFromBytes(char* b, int length)
{
   int rval = 0;
   
   // make sure there are at least 10 bytes available -- this is
   // the minimum header size
   if(length < 10)
   {
      rval = 10 - length;
   }
   else
   {
      // create ByteBuffer for reading header
      ByteBuffer bb(b, 0, length, false);
      
      // store the total number of header bytes needed
      int headerSize = 10;
      
      // ensure ID1 and ID2 are valid
      if(bb.next() != GZIP_ID1 || bb.next() != GZIP_ID2)
      {
         Exception* e = new Exception(
            "Data is not in gzip format!",
            "db.compress.gzip.InvalidHeader");
         Exception::setLast(e);
      }
      else
      {
         // ensure compression method is DEFLATE (CM = 8)
         if(bb.next() != 0x08)
         {
            Exception* e = new Exception(
               "Data is not compressed using DEFLATE!",
               "db.compress.gzip.InvalidCompressionMethod");
            Exception::setLast(e);
         }
         else
         {
            // get flags
            unsigned char flags = bb.next();
            
            // skip rest of header (modification time, extra flags, and
            // operating system)
            // indices 4-7, 8, 9 (6 bytes)
            bb.clear(6);
            
            // check to see if extra flag is set
            if((flags & GZIP_FEXTRA) == GZIP_FEXTRA) 
            {
               // 2 bytes for extra field length
               headerSize += 2;
               if(bb.length() >= 2)
               {
                  // get the extra field length
                  unsigned short xlen = (bb.next() << 8) | bb.next();
                  headerSize += xlen;
                  
                  if(bb.length() >= xlen)
                  {
                     // store the extra field
                     mExtraField.clear();
                     mExtraField.put(bb.data(), xlen, true);
                  }
               }
            }
            
            if((flags & GZIP_FNAME) == GZIP_FNAME)
            {
               // at least one byte for filename
               headerSize++;
               mHasFilename = true;
               
               if(bb.length() >= 1)
               {
                  // try to read in filename
                  string filename;
                  unsigned char c;
                  while(bb.length() > 0 && (c = bb.next()) != 0)
                  {
                     filename.push_back(c);
                  }
                  
                  if(c == 0)
                  {
                     // filename read successfully
                     if(mFilename != NULL)
                     {
                        free(mFilename);
                     }
                     mFilename = strdup(filename.c_str());
                  }
                  else
                  {
                     // need more bytes for filename
                     headerSize += filename.length() + 1;
                  }
               }
            }
            
            if((flags & GZIP_FCOMMENT) == GZIP_FCOMMENT)
            {
               // at least one byte for file comment
               headerSize++;
               mHasFileComment = true;
               
               if(bb.length() >= 1)
               {
                  // try to read in file comment
                  string comment;
                  unsigned char c;
                  while(bb.length() > 0 && (c = bb.next()) != 0)
                  {
                     comment.push_back(c);
                  }
                  
                  if(c == 0)
                  {
                     // file comment read successfully
                     if(mFileComment != NULL)
                     {
                        free(mFileComment);
                     }
                     mFileComment = strdup(comment.c_str());
                  }
                  else
                  {
                     // need more bytes for file comment
                     headerSize += comment.length() + 1;
                  }
               }
            }
            
            if(rval == 0 && (flags & GZIP_FHCRC) == GZIP_FHCRC)
            {
               // add at least 2 bytes for the CRC-16
               headerSize += 2;
               mHasCrc = true;
               
               if(bb.length() >= 2)
               {
                  // check crc
                  mCrc = (bb.next() << 8) | bb.next();
                  unsigned int crc = 0;
                  crc32(crc, (unsigned char*)b, headerSize - 2);
                  
                  // get least significant 2 bytes for crc-16
                  crc = (crc & 0xffff);
                  
                  if(mCrc != crc)
                  {
                     Exception* e = new Exception(
                        "Bad CRC in gzip header!",
                        "db.compress.gzip.BadHeaderCrc");
                     Exception::setLast(e);
                  }
               }
            }
            
            // set the required number of bytes
            if(length < headerSize)
            {
               rval = headerSize - length;
            }
         }
      }
   }
   
   return rval;
}

void Header::convertToBytes(ByteBuffer* b)
{
   // get the size of the header
   int headerSize = 10;
   if(mHasCrc)
   {
      headerSize += 2;
   }
   
   // write ID bytes
   b->put(GZIP_ID1, true);
   b->put(GZIP_ID2, true);
   
   // write the CM (compression method) byte
   b->put(0x08, true);
   
   // write the flag byte
   if(mHasCrc)
   {
      // write crc flag
      b->put(GZIP_FHCRC, true);
   }
   else
   {
      // no flags set
      b->put(0x00, true);
   }
   
   // get the current time as the modification time
   unsigned int time = System::getCurrentMilliseconds() / 1000;
   
   // write the MTIME (modification time)
   b->put((char*)time, 4, true);
   
   // write the XFL (extra flags), no extra flags
   b->put(0x00, true);
   
   // write the OS byte
   b->put(mFileSystemFlag, true);
   
   // see if crc-16 is required
   if(mHasCrc)
   {
      unsigned int crc = 0;
      crc32(crc, b->udata(), 10);
      
      // get the least significant bytes for the crc-16 value
      mCrc = (crc & 0xffff);
      
      // write the crc-16
      b->put((char*)&mCrc, 2, true);
   }
}

void Header::setHasCrc(bool flag)
{
   mHasCrc = flag;
}

void Header::setFileSystemFlag(unsigned char flag)
{
   mFileSystemFlag = flag;
}
