/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/gzip/Gzipper.h"

using namespace db::compress::deflate;
using namespace db::compress::gzip;
using namespace db::io;
using namespace db::rt;

Gzipper::Gzipper()
{
   // create default header and trailer
   mHeader = new Header();
   mTrailer = new Trailer();
   mCleanupHeader = true;
   mCleanupTrailer = true;
   mHeaderProcessed = false;
   mTrailerProcessed = false;
   mCrc32 = 0;
   mGzipFinished = false;
}

Gzipper::~Gzipper()
{
   // clean up header as necessary
   if(mHeader != NULL && mCleanupHeader)
   {
      delete mHeader;
   }
   
   // clean up trailer as necessary
   if(mTrailer != NULL && mCleanupTrailer)
   {
      delete mTrailer;
   }
}

void Gzipper::setHeader(Header* header, bool cleanup)
{
   // clean up header as necessary
   if(mHeader != NULL && mCleanupHeader)
   {
      delete mHeader;
   }
   
   mHeader = header;
   mCleanupTrailer = cleanup;
}

Header* Gzipper::getHeader()
{
   return mHeader;
}

void Gzipper::setTrailer(Trailer* trailer, bool cleanup)
{
   // clean up trailer as necessary
   if(mTrailer != NULL && mCleanupTrailer)
   {
      delete mTrailer;
   }
   
   mTrailer = trailer;
   mCleanupTrailer = cleanup;
}

Trailer* Gzipper::getTrailer()
{
   return mTrailer;
}

bool Gzipper::startCompressing(int level)
{
   // header/trailer not processed yet
   mHeaderProcessed = false;
   mTrailerProcessed = false;
   mBuffer.clear();
   mCrc32 = 0;
   mGzipFinished = false;
   
   // reset header and trailer CRCs
   mHeader->resetCrc();
   mTrailer->setCrc32(0);
   
   // start raw deflation
   return startDeflating(level, true);
}

bool Gzipper::startDecompressing()
{
   // header/trailer not processed yet
   mHeaderProcessed = false;
   mTrailerProcessed = false;
   mBuffer.clear();
   mCrc32 = 0;
   mGzipFinished = false;
   
   // reset header and trailer CRCs
   mHeader->resetCrc();
   mTrailer->setCrc32(0);
   
   // start raw inflation
   return startInflating(true);
}

void Gzipper::setInput(const char* b, int length, bool finish)
{
   // handle reading in the header/trailer
   if(!mDeflating && (!mHeaderProcessed || Deflater::isFinished()))
   {
      // reading header/trailer, append data to cache
      mBuffer.put(b, length, true);
      mShouldFinish = finish;
   }
   else
   {
      // either writing header/trailer, or handling content
      Deflater::setInput(b, length, finish);
   }
}

int Gzipper::process(ByteBuffer* dst, bool resize)
{
   int rval = 0;
   
   if(!mHeaderProcessed)
   {
      // determine if reading or writing the header
      if(mDeflating)
      {
         // write out header
         if(dst->freeSpace() >= mHeader->getSize() || resize)
         {
            mHeader->convertToBytes(dst);
            mHeaderProcessed = true;
            rval = mHeader->getSize();
         }
      }
      else
      {
         // try to convert header
         int ret = mHeader->convertFromBytes(mBuffer.data(), mBuffer.length());
         if(ret == 0)
         {
            // header converted, set input to remaining data
            mHeaderProcessed = true;
            mBuffer.clear(mHeader->getSize());
            setInput(mBuffer.data(), mBuffer.length(), mShouldFinish);
         }
         else if(ret == -1)
         {
            // exception occurred
            rval = -1;
         }
      }
   }
   else if(!Deflater::isFinished())
   {
      // store input (for crc calculation)
      unsigned char* in = mZipStream.next_in;
      unsigned int inLength = mZipStream.avail_in;
      
      // do deflation/inflation
      rval = Deflater::process(dst, resize);
      if(rval != -1)
      {
         // update crc
         if(mDeflating)
         {
            // do calculation on incoming data
            mCrc32 = crc32(mCrc32, in, inLength - mZipStream.avail_in);
         }
         else
         {
            // do calculation on outgoing data
            mCrc32 = crc32(mCrc32, dst->udata() + dst->length() - rval, rval); 
         }
         
         // check for completed inflation
         if(!mDeflating && Deflater::isFinished())
         {
            // clear buffer, append any extra data to it for reading trailer
            mBuffer.clear();
            mBuffer.put((char*)mZipStream.next_in, mZipStream.avail_in, true);
            Deflater::setInput(NULL, 0, mShouldFinish);
         }
      }
   }
   else if(!mTrailerProcessed)
   {
      // determine if reading or writing the trailer
      if(mDeflating)
      {
         // write out 8-byte trailer
         if(dst->freeSpace() >= 8 || resize)
         {
            mTrailer->setCrc32(mCrc32);
            mTrailer->setInputSize(getTotalInputBytes());
            mTrailer->convertToBytes(dst);
            mTrailerProcessed = true;
            rval = 8;
            mGzipFinished = true;
         }
      }
      else
      {
         // try to convert trailer
         int ret = mTrailer->convertFromBytes(mBuffer.data(), mBuffer.length());
         if(ret == 0)
         {
            // trailer converted
            mTrailerProcessed = true;
            mBuffer.clear();
            mGzipFinished = true;
            
            // do crc check
            if(mCrc32 != mTrailer->getCrc32())
            {
               Exception* e = new Exception(
                  "Bad gzip CRC!", "db.compress.gzip.BadCrc");
               Exception::setLast(e);
               rval = -1;
            }
         }
         else if(ret == -1)
         {
            // exception occurred
            rval = -1;
         }
      }
   }
   
   return rval;
}

bool Gzipper::inputAvailable()
{
   return Deflater::inputAvailable() + mBuffer.length();
}

bool Gzipper::isFinished()
{
   return mGzipFinished;
}
