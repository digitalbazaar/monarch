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
    * Set to true when the end of the stream has been reached.
    */
   protected boolean mEndOfStream;
   
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
      
      // end of stream not reached
      mEndOfStream = false;
   }
   
   /**
    * Fills the deflater with uncompressed data from the underlying
    * input stream.
    * 
    * @return true if the deflater was data to read, false if not.
    * 
    * @throws IOException
    */
   protected boolean fillDeflater() throws IOException
   {
      boolean rval = false;
      
      // keep reading while deflater is not finished, needs input, and
      // input is available
      int count = 0;
      while(!getDeflater().finished() && getDeflater().needsInput() &&
            !mEndOfStream)
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
            mEndOfStream = true;
         }
      }
      
      // if the end of the stream has been reached then finish the deflater
      if(mEndOfStream)
      {
         // end of input stream, so finish the deflater
         getDeflater().finish();

         // return true
         rval = true;
      }
      else
      {
         // set return value based on whether or not the deflater needs input
         rval = !getDeflater().needsInput();
      }
      
      return rval;
   }
   
   /**
    * Fills the deflated bytes buffer. The passed target is the target
    * number of bytes to have in the buffer.
    * 
    * @param target the target number of bytes to have in the buffer.
    * 
    * @return true if the bytes buffer was filled with some data, false if
    *         the end of the stream was reached.
    *         
    * @throws IOException
    */
   protected boolean fillDeflatedBytesBuffer(int target) throws IOException
   {
      boolean rval = false;
      
      // if the deflated bytes buffer is used up, reset it
      if(mValidDeflatedBytes == 0)
      {
         mDeflatedBytesReadPosition = 0;
      }
      
      // maximum target is to fill the deflated bytes buffer
      target = Math.min(mDeflatedBytes.length - mValidDeflatedBytes, target);
      
      // keep reading while target is not reached and deflater is not finished
      while(target > 0 && !getDeflater().finished())
      {
         // fill the deflater
         if(fillDeflater())
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
      
      return rval;
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
   public int read(byte b[], int off, int len) throws IOException
   {
      int rval = -1;
      
      // not end of stream if the deflater isn't finished
      if(!getDeflater().finished())
      {
         // fill the deflated bytes buffer
         fillDeflatedBytesBuffer(len);
         
         // read from the deflated bytes buffer
         if(mValidDeflatedBytes > 0)
         {
            rval = Math.min(len, mValidDeflatedBytes);
            
            // copy deflated bytes into passed buffer
            System.arraycopy(mDeflatedBytes, mDeflatedBytesReadPosition,
               b, off, rval);
            
            // update read position and valid bytes
            mDeflatedBytesReadPosition += rval;
            mValidDeflatedBytes -= rval;
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
   public long skip(long n) throws IOException
   {
      long rval = 0;
      
      if(n != 0)
      {
         int numBytes = 0;
         
         int bufferSize = 2048;
         byte[] buffer = new byte[bufferSize];
         
         // read through stream until n reached, discarding data
         while(numBytes != -1 && rval != n)
         {
            // get the number of bytes to read
            int readSize = Math.min((int)(n - rval), bufferSize);
            
            numBytes = read(buffer, 0, readSize);
            
            if(numBytes > 0)
            {                  
               rval += numBytes;
            }
         }
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
   public int available() throws IOException
   {
      int rval = 0;
      
      // return 1 if the deflater isn't finished
      if(!getDeflater().finished())
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
}
