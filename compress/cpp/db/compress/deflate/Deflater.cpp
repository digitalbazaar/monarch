/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/deflate/Deflater.h"

using namespace db::compress::deflate;
using namespace db::io;
using namespace db::rt;

Deflater::Deflater()
{
   mFinished = false;
}

Deflater::~Deflater()
{
   Deflater::cleanupStream();
}

void Deflater::cleanupStream()
{
   // clean up previous stream
   if(mFinished)
   {
      if(mDeflating)
      {
         deflateEnd(&mZipStream);
      }
      else
      {
         inflateEnd(&mZipStream);
      }
   }
   
   // re-initialize stream (use defaults via Z_NULL)
   mZipStream.zalloc = Z_NULL;
   mZipStream.zfree = Z_NULL;
   mZipStream.opaque = Z_NULL;
   mZipStream.next_in = Z_NULL;
   mZipStream.avail_in = 0;
}

Exception* Deflater::createException(int ret)
{
   Exception* rval = NULL;
   
   switch(ret)
   {
      case Z_OK:
      case Z_STREAM_END:
         // success, no exception
         break;
      case Z_BUF_ERROR:
         // not enough space in the buffers
         rval = new Exception(
            "Not enough buffer space!",
            "db.compress.deflate.InsufficientBufferSpace", 0);
         Exception::setLast(rval);
      case Z_MEM_ERROR:
         // not enough memory
         rval = new Exception(
            "Not enough memory for inflation/deflation!",
            "db.compress.deflate.InsufficientMemory", 1);
         Exception::setLast(rval);
         break;
      case Z_VERSION_ERROR:
         // zlib library version incompatible
         rval = new Exception(
            "Incompatible zlib library version!",
            "db.compress.deflate.IncompatibleVersion", 2);
         Exception::setLast(rval);
         break;
      case Z_STREAM_ERROR:
         // invalid stream parameters
         rval = new Exception(
            "Invalid zip stream parameters! Null pointer?",
            "db.compress.deflate.InvalidZipStreamParams", 3);
         Exception::setLast(rval);
         break;
   }
   
   if(rval != NULL && mZipStream.msg != NULL)
   {
      // use zlib stream error message as cause
      rval->setCause(new Exception(mZipStream.msg), true);
   }
   
   return rval;
}

bool Deflater::startDeflating(int level, bool raw)
{
   // clean up previous stream
   cleanupStream();
   
   int ret;
   
   if(raw)
   {
      // windowBits is negative to indicate a raw stream
      // 8 is the default memLevel, use default strategy as well
      ret = deflateInit2(
         &mZipStream, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
   }
   else
   {
      // windowBits set to use zlib header/trailer
      // 8 is the default memLevel, use default strategy as well
      ret = deflateInit2(
         &mZipStream, level, Z_DEFLATED, 15, 8, Z_DEFAULT_STRATEGY);
   }
   
   mDeflating = true;
   mFinished = false;
   
   return (createException(ret) == NULL);
}

bool Deflater::startInflating(bool raw)
{
   // clean up previous stream
   cleanupStream();
   
   int ret;
   
   if(raw)
   {
      // windowBits is negative to indicate a raw stream
      ret = inflateInit2(&mZipStream, -15);
   }
   else
   {
      // add 32 to windowBits to support gzip or zlib decoding
      ret = inflateInit2(&mZipStream, 15 + 32);
   }
   
   mDeflating = false;
   mFinished = false;
   
   return (createException(ret) == NULL);
}

bool Deflater::update(const char* b, int length, ByteBuffer* dest, bool resize)
{
   bool rval = true;
   
   // increase buffer size to at least twice input size if permitted
   dest->allocateSpace(length * 2, resize);
   
   // set output buffer, store old free space
   unsigned int freeSpace = dest->freeSpace();
   mZipStream.next_out = (unsigned char*)dest->data();
   mZipStream.avail_out = freeSpace;
   
   if(mZipStream.avail_in > 0)
   {
      // do update on old data before proceeding
      unsigned int length2 = mZipStream.avail_in;
      mZipStream.avail_in = 0;
      rval = update((char*)mZipStream.next_in, length2, dest, resize);
   }
   
   if(rval)
   {
      // set input buffer and available data
      mZipStream.next_in = (unsigned char*)b;
      mZipStream.avail_in = length;
      
      int ret;
      
      if(mDeflating)
      {
         // do not force flushing, allow zlib to determine when to flush in
         // order to maximize compression
         ret = ::deflate(&mZipStream, Z_NO_FLUSH);
      }
      else
      {
         // do not force flushing, allow zlib to determine when to flush in
         // order to maximize compression
         ret = ::inflate(&mZipStream, Z_NO_FLUSH);
      }
      
      // extend destination buffer by data written to it
      dest->extend(freeSpace - mZipStream.avail_out);
      
      rval = (createException(ret) == NULL);
   }
   
   return rval;
}

bool Deflater::finish(ByteBuffer* dest, bool resize)
{
   int ret = Z_STREAM_END;
   
   if(!mFinished)
   {
      // try to allocate some free space
      dest->allocateSpace(mZipStream.avail_in * 2, true);
      
      // set output buffer, store old free space
      unsigned int freeSpace = dest->freeSpace();
      mZipStream.next_out = (unsigned char*)dest->data();
      mZipStream.avail_out = freeSpace;
      
      if(mDeflating)
      {
         // finish deflating
         ret = ::deflate(&mZipStream, Z_FINISH);
      }
      else
      {
         // finish inflating
         ret = ::inflate(&mZipStream, Z_FINISH);
      }
      
      // extend destination buffer by data written to it
      dest->extend(freeSpace - mZipStream.avail_out);
      
      if(ret == Z_OK)
      {
         if(resize)
         {
            // do finish again with a larger buffer
            dest->allocateSpace(dest->length() * 2, true);
            finish(dest, true);
         }
         else
         {
            // not enough space in the buffer
            ret = Z_BUF_ERROR;
         }
      }
      else
      {
         // now finished
         mFinished = true;
      }
   }
   
   return (createException(ret) == NULL);
}

int Deflater::mutateData(ByteBuffer* src, ByteBuffer* dest, bool finish)
{
   int rval = 1;
   
   if(!mFinished)
   {
      if(src->isEmpty() && !finish)
      {
         // not enough data
         rval = 0;
      }
      else if(!src->isEmpty())
      {
         // update deflation/inflation
         rval = (update(src->data(), src->length(), dest, true) ? 1 : -1);
         src->clear();
      }
      
      if(finish)
      {
         // finish deflation/inflation
         rval = (this->finish(dest, true) ? 1 : -1);
      }
   }
   
   return rval;
}

unsigned long long Deflater::getTotalInputBytes()
{
   return mZipStream.total_in;
}

unsigned long long Deflater::getTotalOutputBytes()
{
   return mZipStream.total_out;
}
