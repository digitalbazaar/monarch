/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/ByteBuffer.h"

#include <cstdlib>
#include <cstring>

using namespace db::io;

ByteBuffer::ByteBuffer(int capacity)
{
   // create the byte buffer
   mCapacity = capacity;
   mBuffer = (capacity > 0) ? (unsigned char*)malloc(mCapacity) : NULL;
   mOffset = 0;
   mLength = 0;
   mCleanup = true;
}

ByteBuffer::ByteBuffer(char* b, int offset, int length, bool cleanup)
{
   // set the byte buffer
   mCleanup = false;
   ByteBuffer::setBytes(b, offset, length, cleanup);
}

ByteBuffer::ByteBuffer(const ByteBuffer& copy)
{
   // copy bytes
   mCapacity = copy.capacity();
   mBuffer = (mCapacity > 0) ? (unsigned char*)malloc(mCapacity) : NULL;
   memcpy(mBuffer, copy.bytes(), copy.capacity());
   mOffset = copy.offset();
   mLength = copy.length();
   mCleanup = true;
}

ByteBuffer::~ByteBuffer()
{
   // clean up byte buffer
   cleanupBytes();
}

void ByteBuffer::cleanupBytes()
{
   if(mCleanup && mBuffer != NULL)
   {
      ::free(mBuffer);
      mBuffer = NULL;
   }
}

void ByteBuffer::free()
{
   cleanupBytes();
   mBuffer = NULL;
   mCapacity = 0;
   mOffset = 0;
   mLength = 0;
   mCleanup = true;
}

void ByteBuffer::allocateSpace(int length, bool resize)
{
   if(resize)
   {
      // determine if the buffer needs to be resized
      int overflow = length - freeSpace();
      if(overflow > 0)
      {
         // resize the buffer by the overflow amount
         this->resize(mCapacity + overflow);
      }
   }
   
   // determine if the data needs to be shifted
   if(mOffset > 0)
   {
      int overflow = length - freeSpace() + mOffset;
      if(overflow > 0)
      {
         if(mLength > 0)
         {
            // shift the data in the buffer
            memmove(mBuffer, udata(), mLength);
         }
         
         mOffset = 0;
      }
   }
}

void ByteBuffer::resize(int capacity)
{
   if(capacity != mCapacity)
   {
      if(mCleanup && mBuffer != NULL)
      {
         // move existing data to front of buffer
         if(mOffset != 0)
         {
            memmove(mBuffer, udata(), mLength);
            mOffset = 0;
         }
         
         // reallocate buffer
         mBuffer = (unsigned char*)realloc(mBuffer, capacity);
         mCapacity = capacity;
         mLength = (mCapacity < mLength) ? mCapacity : mLength;
         mOffset = (mOffset < mLength) ? mOffset : mLength;
      }
      else
      {
         // create a new buffer
         unsigned char* newBuffer = (unsigned char*)malloc(capacity);
         
         // copy the data into the new buffer, truncate old count as necessary
         mCapacity = capacity;
         mLength = (mCapacity < mLength) ? mCapacity : mLength;
         memcpy(newBuffer, udata(), mLength);
         mOffset = 0;
         
         // clean up old buffer
         cleanupBytes();
         
         // memory management now on regardless of previous setting
         mBuffer = newBuffer;
         mCleanup = true;
      }
   }
}

int ByteBuffer::put(unsigned char b, bool resize)
{
   int rval = 0;
   
   // allocate space for the data
   allocateSpace(1, resize);
   
   if(freeSpace() > 0)
   {
      // put byte into the buffer
      udata()[mLength++] = b;
      rval++;
   }
   
   return rval;
}

int ByteBuffer::put(const char* b, int length, bool resize)
{
   // allocate space for the data
   allocateSpace(length, resize);
   
   // copy data into the buffer
   length = (length < freeSpace()) ? length : freeSpace();
   
   if(length < 10)
   {
      // optimized over memcpy()
      unsigned char* ub = (unsigned char*)b;
      for(int i = 0; i < length; i++)
      {
         udata()[mLength + i] = ub[i];
      }
   }
   else
   {
      memcpy(udata() + mLength, b, length);
   }
   
   mLength += length;
   
   return length;
}

int ByteBuffer::put(ByteBuffer* b, int length, bool resize)
{
   length = (length < b->length()) ? length : b->length();
   return put(b->bytes() + b->offset(), length, resize);
}

int ByteBuffer::put(InputStream* is, int length)
{
   int rval = 0;
   
   // if the buffer is not full, do a read
   if(!isFull())
   {
      // allocate free space
      allocateSpace(freeSpace(), false);
      
      // determine how much to read
      length = (length > 0 && length < freeSpace() ? length : freeSpace());
      
      // read
      rval = is->read(data() + mLength, length);
      if(rval != -1)
      {
         // increment length
         mLength += rval;
      }
   }
   
   return rval;
}

int ByteBuffer::get(unsigned char& b)
{
   int rval = 0;
   
   if(mLength > 0)
   {
      // get byte
      b = udata()[0];
      
      // move internal pointer
      mOffset++;
      mLength--;
      rval++;
   }
   
   return rval;
}

int ByteBuffer::get(char* b, int length)
{
   length = (length < mLength) ? length : mLength;
   memcpy(b, data(), length);
   
   // move internal pointer
   mOffset += length;
   mLength -= length;
   
   return length;
}

int ByteBuffer::get(ByteBuffer* b, int length, bool resize)
{
   // put data into passed buffer
   length = (length < mLength) ? length : mLength;
   int rval = b->put(data(), length, resize);
   
   // move internal pointer and change length
   mOffset += rval;
   mLength -= rval;
   
   return rval;
}

int ByteBuffer::get(OutputStream* os)
{
   int rval = 0;
   
   if(os->write(data(), mLength))
   {
      rval = mLength;
      mOffset = mLength = 0;
   }
   
   return rval;
}

int ByteBuffer::clear(int length)
{
   // ensure that the maximum cleared is existing length
   int rval = (length > 0) ? ((mLength < length) ? mLength : length) : 0;
   
   // set new length and offset
   mLength -= rval;
   mOffset = (mLength == 0) ? 0 : mOffset + rval;
   
   return rval;
}

int ByteBuffer::clear()
{
   return clear(mLength);
}

int ByteBuffer::reset(int length)
{
   // ensure that the most the offset is moved back is the existing offset
   int rval = (length > 0) ? ((mOffset < length) ? mOffset : length) : 0;
   
   // set new offset and length
   mOffset -= rval;
   mLength += rval;
   
   return rval;
}

int ByteBuffer::trim(int length)
{
   // ensure that the maximum trimmed is existing length
   int rval = (length > 0) ? ((mLength < length) ? mLength : length) : 0;
   
   // set new length
   mLength -= rval;
   
   return rval;
}

int ByteBuffer::extend(int length)
{
   // ensure that the maximum extended is (free space - offset)
   int max = freeSpace() - mOffset;
   int rval = (length > 0) ? ((max < length) ? max : length) : 0;
   
   // set new length
   mLength += rval;
   
   return rval;
}

unsigned char ByteBuffer::next()
{
   mLength--;
   mOffset++;
   return (udata() - 1)[0];
}

int ByteBuffer::capacity() const
{
   return mCapacity;
}

void ByteBuffer::setBytes(ByteBuffer* b, bool cleanup)
{
   // set the byte buffer
   setBytes(b->bytes(), b->offset(), b->length(), cleanup);
}

void ByteBuffer::setBytes(char* b, int offset, int length, bool cleanup)
{
   // cleanup old buffer
   cleanupBytes();
   
   mCapacity = length;
   mBuffer = (unsigned char*)b;
   mOffset = offset;
   mLength = length;
   mCleanup = cleanup;
}

inline char* ByteBuffer::bytes() const
{
   return (char*)mBuffer;
}

inline unsigned char* ByteBuffer::ubytes() const
{
   return mBuffer;
}

inline char* ByteBuffer::data() const
{
   return (char*)(mBuffer + mOffset);
}

inline unsigned char* ByteBuffer::udata() const
{
   return mBuffer + mOffset;
}

inline int ByteBuffer::offset() const
{
   return mOffset;
}

inline int ByteBuffer::length() const
{
   return mLength;
}

inline int ByteBuffer::freeSpace() const
{
   return mCapacity - mLength;
}

inline bool ByteBuffer::isFull() const
{
   return freeSpace() == 0;
}

inline bool ByteBuffer::isEmpty() const
{
   return mLength == 0;
}

inline bool ByteBuffer::isManaged() const
{
   return mCleanup;
}
