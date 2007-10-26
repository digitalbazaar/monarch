/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/ByteBuffer.h"

using namespace db::io;

ByteBuffer::ByteBuffer(int capacity)
{
   // create the byte buffer
   mCapacity = capacity;
   mBuffer = (capacity > 0) ? new char[mCapacity] : NULL;
   mOffset = 0;
   mLength = 0;
   mCleanup = true;
}

ByteBuffer::ByteBuffer(char* b, int offset, int length, bool cleanup)
{
   // set the byte buffer
   setBytes(b, offset, length, cleanup);
}

ByteBuffer::ByteBuffer(const ByteBuffer& copy)
{
   // copy bytes
   mCapacity = copy.getCapacity();
   mBuffer = (mCapacity > 0) ? new char[mCapacity] : NULL;
   memcpy(mBuffer, copy.bytes(), copy.getCapacity());
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
      delete [] mBuffer;
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
      int overflow = length - getFreeSpace();
      if(overflow > 0)
      {
         // resize the buffer by the overflow amount
         this->resize(mCapacity + overflow);
      }
   }
   
   // determine if the data needs to be shifted
   if(mOffset > 0)
   {
      int overflow = length - getFreeSpace() + mOffset;
      if(overflow > 0)
      {
         if(mLength > 0)
         {
            // shift the data in the buffer
            memcpy(mBuffer, data(), mLength);
         }
         
         mOffset = 0;
      }
   }
}

void ByteBuffer::resize(int capacity)
{
   if(capacity != mCapacity)
   {
      // create a new buffer
      char* newBuffer = new char[capacity];
      
      // copy the data into the new buffer, truncate old count as necessary
      mCapacity = capacity;
      mLength = (mCapacity < mLength) ? mCapacity : mLength;
      memcpy(newBuffer, data(), mLength);
      mOffset = 0;
      
      // clean up old buffer
      cleanupBytes();
      
      // memory management now on regardless of previous setting
      mBuffer = newBuffer;
      mCleanup = true;
   }
}

int ByteBuffer::put(const char* b, int length, bool resize)
{
   // allocate space for the data
   allocateSpace(length, resize);
   
   // copy data into the buffer
   length = (length < getFreeSpace()) ? length : getFreeSpace();
   memcpy(data() + mLength, b, length);
   mLength += length;
   
   return length;
}

int ByteBuffer::put(ByteBuffer* b, int length, bool resize)
{
   length = (length < b->length()) ? length : b->length();
   return put(b->bytes() + b->offset(), length, resize);
}

int ByteBuffer::put(InputStream* is)
{
   int rval = 0;
   
   // if the buffer is not full, do a read
   if(!isFull())
   {
      // allocate free space
      allocateSpace(getFreeSpace(), false);
      
      // read
      rval = is->read(data() + mLength, getFreeSpace());
      if(rval != -1)
      {
         // increment length
         mLength += rval;
      }
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
   int rval = mLength;
   
   os->write(data(), mLength);
   mOffset = mLength = 0;
   
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
   int max = getFreeSpace() - mOffset;
   int rval = (length > 0) ? ((max < length) ? max : length) : 0;
   
   // set new length
   mLength += rval;
   
   return rval;
}

int ByteBuffer::getCapacity() const
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
   
   mBuffer = b;
   mOffset = offset;
   mLength = length;
   mCleanup = cleanup;
}

char* ByteBuffer::bytes() const
{
   return mBuffer;
}

char* ByteBuffer::data() const
{
   return mBuffer + mOffset;
}

int ByteBuffer::offset() const
{
   return mOffset;
}

int ByteBuffer::length() const
{
   return mLength;
}

int ByteBuffer::getFreeSpace() const
{
   return mCapacity - mLength;
}

bool ByteBuffer::isFull() const
{
   return getFreeSpace() == 0;
}

bool ByteBuffer::isEmpty() const
{
   return mLength == 0;
}

bool ByteBuffer::isManaged() const
{
   return mCleanup;
}
