/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/deflate/Deflater.h"

using namespace db::compress::deflate;
using namespace db::io;
using namespace db::rt;

// Note: chunk sizes of 128K or 256K are recommended by zlib (if possible)
#define CHUNK_SIZE 16384

Deflater::Deflater()
{
   mShouldFinish = false;
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
      case Z_BUF_ERROR:
         // success or not enough buffer space (not fatal), so no exception
         break;
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
      default:
         // something else went wrong
         rval = new Exception(
            "Could not inflate/deflate!",
            "db.compress.deflate.Error", 4);
         Exception::setLast(rval);
         break;
   }
   
   if(rval != NULL)
   {
      if(mZipStream.msg != NULL)
      {
         // use zlib stream error message as cause
         rval->setCause(new Exception(mZipStream.msg), true);
      }
      
      // set last exception
      Exception::setLast(rval);
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
   mShouldFinish = false;
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
   mShouldFinish = false;
   mFinished = false;
   
   return (createException(ret) == NULL);
}

void Deflater::setInput(const char* b, int length, bool finish)
{
   // set zip stream input buffer
   mZipStream.next_in = (unsigned char*)b;
   mZipStream.avail_in = length;
   mShouldFinish = finish;
}

int Deflater::process(ByteBuffer* dst, bool resize)
{
   int rval = 0;
   
   if(!mFinished)
   {
      // when deflating, let zlib determine flushing to maximize compression
      // when inflating, flush output whenever possible
      int flush = (mShouldFinish) ?
         Z_FINISH : ((mDeflating) ? Z_NO_FLUSH : Z_SYNC_FLUSH);
      
      // keep processing while no error, no output data, while there is
      // input data or processing should finish, and while there is room
      // to store the output data or if the destination buffer can be
      // resized
      while(rval == 0 &&
            (mZipStream.avail_in > 0 || mShouldFinish) &&
            (resize || !dst->isFull()))
      {
         if(resize && dst->isFull())
         {
            // allocate space for output
            dst->allocateSpace(1024, resize);
         }
         
         // set output buffer, store old free space
         int freeSpace = dst->freeSpace();
         mZipStream.next_out = (unsigned char*)dst->data();
         mZipStream.avail_out = freeSpace;
         
         // perform deflation/inflation
         int ret = (mDeflating) ?
            ::deflate(&mZipStream, flush) :
            ::inflate(&mZipStream, flush);
         
         // extend destination buffer by data written to it
         int length = freeSpace - mZipStream.avail_out;
         dst->extend(length);
         
         // handle potential exception
         if(createException(ret) != NULL)
         {
            rval = -1;
         }
         else
         {
            // return number of output bytes
            rval = length;
         }
         
         if(ret == Z_STREAM_END)
         {
            // finished
            mFinished = true;
         }
      }
   }
   
   return rval;
}

MutationAlgorithm::Result Deflater::mutateData(
   ByteBuffer* src, ByteBuffer* dst, bool finish)
{
   MutationAlgorithm::Result rval = MutationAlgorithm::CompleteAppend;
   
   if(!mFinished)
   {
      if(mZipStream.avail_in == 0)
      {
         // set more input
         setInput(src->data(), src->length(), finish);
         if(src->isEmpty() && !finish)
         {
            // more data required
            rval = MutationAlgorithm::NeedsData;
         }
      }
      
      // keep processing while not finished, no error, no output data,
      // and while source data is not empty or while not finishing
      int ret = 0;
      while(!mFinished && ret != -1 &&
            dst->isEmpty() && (!src->isEmpty() || mShouldFinish))
      {
         // try to process existing input
         ret = process(dst, false);
         if(ret == 0)
         {
            // clear source and request more data
            src->clear();
            rval = MutationAlgorithm::NeedsData;
         }
      }
      
      if(ret == -1)
      {
         rval = MutationAlgorithm::Error;
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
