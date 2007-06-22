/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.zip.Deflater;

/**
 * A DeflaterInputStream is used to "deflate" (compress) data as it
 * is read.
 * 
 * @author Dave Longley
 */
public class DeflaterInputStream extends FilterInputStream
{
   /**
    * The deflater (compressor) for this stream.
    */
   protected Deflater mDeflater;
   
   /**
    * A buffer for storing inflated (compressed) bytes from the
    * underlying input stream.
    */
   protected byte[] mInflatedBytes;
   
   /**
    * A buffer for storing deflated bytes.
    */
   protected byte[] mDeflatedBytes;
   
   /**
    * The read position in the deflated bytes buffer.
    */
   protected int mDeflatedBytesReadPosition;
   
   /**
    * The total number of valid bytes left in the deflated bytes buffer.
    */
   protected int mValidDeflatedBytes;
   
   /**
    * A single byte buffer to use when reading one byte via read().
    */
   protected byte[] mSingleByteBuffer;
   
   /**
    * Set to true when the end of the underlying stream has been reached.
    */
   protected boolean mEndOfUnderlyingStream;
   
   /**
    * A buffer for skipping data.
    */
   protected static byte[] mSkipBuffer;
   
   /**
    * Creates a new DeflaterInputStream with a default Deflater and
    * a default buffer size of 2048 bytes.
    * 
    * @param is the input stream to read uncompressed data from.
    */
   public DeflaterInputStream(InputStream is)
   {
      this(is, new Deflater());
   }

   /**
    * Creates a new DeflaterInputStream with a default buffer size of
    * 2048 bytes.
    * 
    * @param is the input stream to read uncompressed data from.
    * @param deflater the deflater to use.
    */
   public DeflaterInputStream(InputStream is, Deflater deflater)
   {
      this(is, deflater, 2048);
   }
   
   /**
    * Creates a new DeflaterInputStream.
    * 
    * @param is the input stream to read uncompressed data from.
    * @param deflater the deflater to use.
    * @param bufferSize the size of the internal deflated bytes buffer.
    * 
    * @throws IllegalArgumentException
    */
   public DeflaterInputStream(InputStream is, Deflater deflater, int bufferSize)
   throws IllegalArgumentException
   {
      // store underlying input stream
      super(is);
      
      // throw exception is buffer size is < 1
      if(bufferSize < 1)
      {
         throw new IllegalArgumentException("bufferSize must be >= 1");
      }
      
      // store deflater
      mDeflater = deflater;
      
      // create the internal read buffer for read inflated bytes
      mInflatedBytes = new byte[2048];

      // create the deflated bytes buffer
      mDeflatedBytes = new byte[bufferSize];
      
      // read position is 0
      mDeflatedBytesReadPosition = 0;
      
      // valid bytes is 0
      mValidDeflatedBytes = 0;
      
      // create the single byte buffer
      mSingleByteBuffer = new byte[1];
      
      // end of underlying stream not reached
      mEndOfUnderlyingStream = false;
   }
   
   /**
    * Fills the deflater with uncompressed data from the underlying
    * input stream.
    * 
    * @throws IOException
    */
   protected void fillDeflater() throws IOException
   {
      // keep reading while not end of underlying stream, and
      // deflater is not finished and needs input
      int count = 0;
      while(!mEndOfUnderlyingStream &&
            !getDeflater().finished() && getDeflater().needsInput())
      {
         int b = in.read();
         if(b != -1)
         {
            if(count == mInflatedBytes.length)
            {
               // increase the read buffer size as necessary
               byte[] newBuffer = new byte[count * 2];
               System.arraycopy(mInflatedBytes, 0, newBuffer, 0, count);
               mInflatedBytes = newBuffer;
            }
            
            // add inflated byte to buffer, increment count afterwards
            mInflatedBytes[count++] = (byte)(b & 0xff);
            
            // set deflater input
            getDeflater().setInput(mInflatedBytes, 0, count);
         }
         else
         {
            // end of underlying input stream reached
            mEndOfUnderlyingStream = true;
         }
      }
      
      // if the end of the underlying stream has been reached then
      // finish the deflater
      if(mEndOfUnderlyingStream)
      {
         // end of input stream, so finish the deflater
         getDeflater().finish();
      }
   }
   
   /**
    * Fills the deflated bytes buffer.
    * 
    * @throws IOException
    */
   protected void fillDeflatedBytesBuffer() throws IOException
   {
      // if the deflated bytes buffer is used up, reset the read position
      if(mValidDeflatedBytes == 0)
      {
         mDeflatedBytesReadPosition = 0;
      }
      
      // target is to fill up the deflated bytes buffer
      int target = mDeflatedBytes.length - mValidDeflatedBytes;
      
      // keep reading while target is not reached or deflater is not finished
      while(target > 0 && !getDeflater().finished())
      {
         // fill the deflater
         fillDeflater();
         
         // deflate data if the deflater isn't finished
         if(!getDeflater().finished())
         {
            // deflate data into deflated bytes buffer
            int count = mDeflater.deflate(
               mDeflatedBytes, mValidDeflatedBytes, target);
         
            // set the deflated bytes read position and valid byte count
            mValidDeflatedBytes += count;
            
            // decrement target
            target -= count;
         }
      }
   }
   
   /**
    * Reads the next byte of this input stream. If the end of the
    * stream has been reached, -1 is returned. This method blocks
    * until some data is available, the end of the stream is
    * reached, or an exception is thrown.
    * 
    * @return the next byte of data or -1 if the end of the stream has
    *         been reached.
    *         
    * @throws IOException
    */
   @Override
   public int read() throws IOException
   {
      int rval = -1;
      
      if(read(mSingleByteBuffer) != -1)
      {
         rval = mSingleByteBuffer[0] & 0xff;
      }
      
      return rval;
   }
   
   /**
    * Reads up to <code>len</code> bytes of data from this input stream 
    * into an array of bytes. This method blocks until some input is 
    * available.
    * 
    * The data that is read from the underlying stream is deflated
    * (compressed) as it is read.
    *
    * @param b the buffer to read the data into.
    * @param off the offset in the buffer to start the read data at.
    * @param len the maximum number of bytes read into the buffer.
    * 
    * @return the number of bytes read into the buffer or -1 if the
    *         end of the stream has been reached.
    *         
    * @throws IOException
    */
   @Override
   public int read(byte b[], int off, int len) throws IOException
   {
      int rval = -1;
      
      // read if this stream still has deflated bytes to read
      if(hasDeflatedBytes())
      {
         // return 0 if length is 0
         if(len == 0)
         {
            rval = 0;
         }
         else
         {
            // read while no data has been read and there is data to read
            rval = 0;
            while(rval == 0 && hasDeflatedBytes())
            {
               // if data is needed, get it
               if(mValidDeflatedBytes == 0)
               {
                  // fill the read deflated bytes buffer
                  fillDeflatedBytesBuffer();
               }
               else
               {
                  // read from the deflated bytes buffer
                  rval = Math.min(len, mValidDeflatedBytes);
               
                  // copy deflated bytes into passed buffer
                  System.arraycopy(mDeflatedBytes, mDeflatedBytesReadPosition,
                     b, off, rval);
               
                  // update read position and valid bytes
                  mDeflatedBytesReadPosition += rval;
                  mValidDeflatedBytes -= rval;
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Skips over and discards <code>n</code> bytes of data from the 
    * input stream. The <code>skip</code> method may, for a variety of 
    * reasons, end up skipping over some smaller number of bytes, 
    * possibly <code>0</code>. The actual number of bytes skipped is 
    * returned.
    *
    * @param n the number of bytes to be skipped.
    * 
    * @return the actual number of bytes skipped.
    * 
    * @throws IOException
    */
   @Override
   public long skip(long n) throws IOException
   {
      long rval = 0;
      
      if(mSkipBuffer == null)
      {
         mSkipBuffer = new byte[2048];
      }
      byte[] b = mSkipBuffer;
      
      // read into dummy buffer
      long remaining = n;
      int numBytes = Math.max(b.length, (int)n);
      while(remaining > 0 && (numBytes = read(b, 0, numBytes)) != -1)
      {
         remaining -= numBytes;
         numBytes = Math.max(b.length, (int)n);
      }
      
      if(remaining < n)
      {
         // some bytes were skipped
         rval = n - remaining;
      }
      
      return rval;
   }
   
   /**
    * Returns how many bytes are available from this input stream.
    * 
    * @return how many bytes are available from this input stream.
    * 
    * @throws IOException
    */
   @Override
   public int available() throws IOException
   {
      int rval = 0;
      
      // return 1 if there are deflated bytes to read
      if(hasDeflatedBytes())
      {
         rval = 1;
      }
      
      return rval;
   }
   
   /**
    * Finishes the deflater. This method should be called if the input
    * stream is in a chain and should not be closed to finish the
    * deflater.
    * 
    * The read() methods for this stream will continue to read until the
    * deflater is empty.
    */
   public void finish()
   {
      // finish the deflater
      getDeflater().finish();
   }
   
   /**
    * Closes this input stream and the underlying input stream. Ends
    * the Deflater.
    * 
    * @throws IOException
    */
   @Override
   public void close() throws IOException
   {
      // close underlying input stream
      in.close();
      
      // finish
      finish();
      
      // end the deflater
      getDeflater().end();
   }
   
   /**
    * Gets the deflater used by this input stream.
    * 
    * @return the deflater used by this input stream.
    */
   public Deflater getDeflater()
   {
      return mDeflater;
   }
   
   /**
    * Returns true if there are deflated bytes that can be read from this
    * stream.
    * 
    * @return true if there are deflated bytes that can be read from this
    *         stream, false if there aren't any more.
    */
   public boolean hasDeflatedBytes()
   {
      boolean rval = true;
      
      // see if there is no more deflater data
      if(getDeflater().finished())
      {
         // see if there are no more valid deflated bytes to read
         if(mValidDeflatedBytes == 0)
         {
            rval = false;
         }
      }
      
      return rval;
   }
}
