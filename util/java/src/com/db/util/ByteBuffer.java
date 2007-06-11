/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.io.IOException;
import java.io.InputStream;

/**
 * A ByteBuffer holds an internal array of bytes that can be dynamically
 * resized, filled, and emptied.
 * 
 * @author Dave Longley
 */
public class ByteBuffer
{
   /**
    * The internal byte buffer. 
    */
   protected byte[] mBuffer;
   
   /**
    * The current offset in the buffer.
    */
   protected int mOffset;
   
   /**
    * The number of valid bytes in the buffer.
    */
   protected int mCount;
   
   /**
    * Creates a new ByteBuffer with the specified starting size.
    * 
    * @param capacity the capacity of the ByteBuffer in bytes.
    */
   public ByteBuffer(int capacity)
   {
      if(capacity < 0)
      {
         throw new IllegalArgumentException("ByteBuffer capacity must be >= 0");
      }
      
      // create the byte buffer
      mBuffer = new byte[capacity];
      mOffset = 0;
      mCount = 0;
   }
   
   /**
    * Allocates enough space in the current buffer for the passed number of
    * bytes.
    * 
    * @param length the number of bytes that need to be written to this buffer.
    * @param resize true to resize the buffer as is necessary, false not to. 
    */
   protected void allocateSpace(int length, boolean resize)
   {
      if(resize)
      {
         // determine if the buffer needs to be resized
         int overflow = length - getFreeSpace();
         if(overflow > 0)
         {
            // resize the buffer by the overflow amount
            resize(mBuffer.length + overflow);
         }
      }
      
      // determine if the data needs to be shifted
      if(mOffset > 0)
      {
         int overflow = length - getFreeSpace() + mOffset;
         if(overflow > 0)
         {
            // shift the data in the buffer
            System.arraycopy(mBuffer, mOffset, mBuffer, 0, mCount);
            mOffset = 0;
         }
      }
   }
   
   /**
    * Resizes the ByteBuffer to the given capacity. Any existing data that
    * cannot fit in the new capacity will be truncated.
    * 
    * @param capacity the new capacity, in bytes, for this buffer.
    */
   public void resize(int capacity)
   {
      if(capacity < 0)
      {
         throw new IllegalArgumentException("ByteBufferSize capacity be >= 0");
      }
      
      // create a new buffer
      byte[] newBuffer = new byte[capacity];
      
      // copy the data into the new buffer, truncate old count as necessary
      mCount = Math.min(capacity, mCount);
      System.arraycopy(mBuffer, mOffset, newBuffer, 0, mCount);
      mOffset = 0;
      mBuffer = newBuffer;
   }
   
   /**
    * Puts data from the passed buffer into this buffer.
    * 
    * @param b the buffer with data to put into this buffer.
    * @param offset the offset at which to start retrieving data.
    * @param length the number of bytes to put into this buffer.
    * 
    * @return the actual number of bytes put into this buffer, which may be
    *         less than the number of bytes requested if this buffer is full.
    */
   public int put(byte[] b, int offset, int length)
   {
      return put(b, offset, length, false);
   }
   
   /**
    * Puts data from the passed buffer into this buffer.
    * 
    * @param b the buffer with data to put into this buffer.
    * @param offset the offset at which to start retrieving data.
    * @param length the number of bytes to put into this buffer.
    * @param resize true to automatically resize this buffer if the passed
    *               number of bytes will not otherwise fit.
    * 
    * @return the actual number of bytes put into this buffer, which may be
    *         less than the number of bytes requested if this buffer is full.
    */
   public int put(byte[] b, int offset, int length, boolean resize)
   {
      // allocate space for the data
      allocateSpace(length, resize);
      
      // copy data into the buffer
      length = Math.min(length, mBuffer.length - (mOffset + mCount));
      System.arraycopy(b, offset, mBuffer, mOffset + mCount, length);
      mCount += length;
      
      return length;
   }
   
   /**
    * Reads data from the passed input stream and puts it into this buffer.
    * 
    * This method will block until at one byte can be read from the input
    * stream, unless this buffer is already full, or until the end of the
    * stream has been reached.
    * 
    * @param is the input stream to read from.
    * 
    * @return the number of bytes read from the input stream and put into
    *         this buffer, or -1 if the end of the input stream was reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public int put(InputStream is) throws IOException
   {
      int rval = -1;
      
      // if the buffer is not full, do a read
      if(!isFull())
      {
         // allocate free space
         allocateSpace(getFreeSpace(), false);
         
         // read
         rval = is.read(mBuffer, mOffset + mCount, getFreeSpace());
         
         if(rval != -1)
         {
            // increment count
            mCount += rval;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets data out of this buffer and puts it into the passed buffer. This
    * method will increment the internal pointer of this buffer by the number
    * of bytes retrieved.
    * 
    * @param b the buffer to put the retrieved data into.
    * @param offset the offset at which to put the retrieved data.
    * @param length the maximum number of bytes to get.
    * 
    * @return the actual number of bytes retrieved, which may be 0 if this
    *         buffer is empty.
    */
   public int get(byte[] b, int offset, int length)
   {
      length = Math.min(length, getUsedSpace());
      System.arraycopy(mBuffer, mOffset, b, offset, length);
      
      // move internal pointer
      mOffset += length;
      mCount -= length;
      
      return length;
   }
   
   /**
    * Gets data out of this buffer and puts it into the passed ByteBuffer. This
    * method will increment the internal pointer of this buffer by the number
    * of bytes retrieved.
    * 
    * @param b the ByteBuffer to put the retrieved data into.
    * @param length the maximum number of bytes to get.
    * @param resize true to resize the passed buffer as necessary, false not to.
    * 
    * @return the actual number of bytes retrieved, which may be 0 if this
    *         buffer is empty or if the passed buffer is full.
    */
   public int get(ByteBuffer b, int length, boolean resize)
   {
      // put data into passed buffer
      int rval = b.put(mBuffer, mOffset, Math.min(mCount, length), resize);
      
      // move internal pointer and change count
      mOffset += rval;
      mCount -= rval;
      
      return rval;
   }
   
   /**
    * Clears the specified amount of data out of this buffer. If more data is
    * requested to be cleared than there is data, all data will be cleared.
    * 
    * @param length the amount of data to clear from this buffer.
    * 
    * @return the actual amount of data cleared.
    */
   public int clear(int length)
   {
      int rval = mCount;
      
      mCount = Math.max(mCount - length, 0);
      
      if(mCount == 0)
      {
         mOffset = 0;
      }
      else
      {
         mOffset += length;
         rval = length;
      }
      
      return rval;
   }
   
   /**
    * Clears all data out of this buffer.
    * 
    * @return the actual amount of data cleared.
    */
   public int clear()
   {
      int rval = mCount;
      
      mCount = 0;
      mOffset = 0;
      
      return rval;
   }
   
   /**
    * Gets the capacity of this buffer.
    * 
    * @return the capacity of this buffer.
    */
   public int getCapacity()
   {
      return mBuffer.length;
   }
   
   /**
    * Gets the amount of used space in this buffer.
    * 
    * @return the amount of used space in this buffer.
    */
   public int getUsedSpace()
   {
      return mCount;
   }
   
   /**
    * Gets the amount of free space in this buffer.
    * 
    * @return the amount of free spaces in this buffer, in bytes.
    */
   public int getFreeSpace()
   {
      return mBuffer.length - mCount;
   }
   
   /**
    * Returns true if this buffer is full.
    * 
    * @return true if this buffer is full, false if it is not.
    */
   public boolean isFull()
   {
      return getFreeSpace() == 0;
   }
   
   /**
    * Returns true if this buffer is empty.
    * 
    * @return true if this buffer is empty, false if it is not.
    */
   public boolean isEmpty()
   {
      return getUsedSpace() == 0;
   }
   
   /**
    * Gets the bytes in this buffer. This method will return the underlying
    * byte array. The offset at which valid bytes begin can be retrieved by
    * calling getOffset().
    * 
    * @return the bytes in this buffer.
    */
   public byte[] getBytes()
   {
      return mBuffer;
   }
   
   /**
    * Gets the offset at which the bytes start in this buffer.
    * 
    * @return the offset at which the bytes start in this buffer.
    */
   public int getOffset()
   {
      return mOffset;
   }
}
