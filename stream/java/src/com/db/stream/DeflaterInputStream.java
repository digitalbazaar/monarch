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
    * The deflater for this stream.
    */
   protected Deflater mDeflater;
   
   /**
    * A buffer for reading from the underlying input stream.
    */
   protected byte[] mReadBuffer;
   
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
    * Gets the number of bytes read from the underlying input stream.
    */
   protected long mUnderlyingBytesRead;
   
   /**
    * Creates a new DeflaterInputStream with a default Deflater.
    * 
    * @param is the input stream to read uncompressed data from.
    */
   public DeflaterInputStream(InputStream is)
   {
      this(is, new Deflater());
   }

   /**
    * Creates a new DeflaterInputStream.
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
    */
   public DeflaterInputStream(InputStream is, Deflater deflater, int bufferSize)
   {
      super(is);
      
      // throw exception is buffer size is <= 0
      if(bufferSize <= 0)
      {
         throw new IllegalArgumentException("bufferSize must be > 0");
      }
      
      // store deflater
      mDeflater = deflater;
      
      // create the interal read buffer
      mReadBuffer = new byte[2048];

      // create the deflated bytes buffer
      mDeflatedBytes = new byte[bufferSize];
      
      // read position is 0
      mDeflatedBytesReadPosition = 0;
      
      // valid bytes is 0
      mValidDeflatedBytes = 0;
      
      // underlying bytes read is 0
      mUnderlyingBytesRead = 0;
   }
   
   /**
    * Fills the deflater with uncompressed data from the underlying
    * input stream.
    * 
    * @return true if the deflater could be filled, false if the
    *         end of the stream was reached and it could not be filled.
    * 
    * @throws IOException
    */
   protected boolean fillDeflater() throws IOException
   {
      boolean rval = false;
      
      // keep reading until the deflater no longer needs input
      int b = 0;
      int count = 0;
      while(!mDeflater.finished() &&
            mDeflater.needsInput() && (b = in.read()) != -1)
      {
         if(count == mReadBuffer.length)
         {
            // increase the read buffer size as necessary
            byte[] newBuffer = new byte[count * 2];
            System.arraycopy(mReadBuffer, 0, newBuffer, 0, count);
            mReadBuffer = newBuffer;
         }
         
         mReadBuffer[count++] = (byte)b;
         
         // set deflater input
         mDeflater.setInput(mReadBuffer, 0, count);
         
         // increment total underlying bytes read
         mUnderlyingBytesRead++;
      }

      if(b == -1)
      {
         // end of input stream, so finish the deflater
         mDeflater.finish();
      }
      
      // set return value based on whether or not deflater needs more input
      rval = !mDeflater.needsInput();
      
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
      
      // throw an exception if the deflater is finished already
      if(mDeflater.finished())
      {
         throw new IOException("Deflator is already finished!");
      }
      
      // read from the deflated bytes if a byte is available
      if(mValidDeflatedBytes > 0)
      {
         rval = mDeflatedBytes[mDeflatedBytesReadPosition++];
         mValidDeflatedBytes--;
      }
      else
      {
         // fill the deflater
         if(fillDeflater())
         {
            // deflate data into deflated bytes buffer
            int count = mDeflater.deflate(mDeflatedBytes);
         
            // update deflated bytes read position, valid byte count
            mDeflatedBytesReadPosition = 1;
            mValidDeflatedBytes = count - 1;
                     
            // get the first deflated byte
            rval = mDeflatedBytes[0];
         }
      }
      
      return rval;
   }
   
   /**
    * Reads up to <code>len</code> bytes of data from this input stream 
    * into an array of bytes. This method blocks until some input is 
    * available. 
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
      
      // throw an exception if the deflater is finished already
      if(mDeflater.finished())
      {
         throw new IOException("Deflator is already finished!");
      }
      
      // read from the deflated bytes if a byte is available
      if(mValidDeflatedBytes > 0)
      {
         rval = Math.min(len, mValidDeflatedBytes);
         System.arraycopy(mDeflatedBytes, mDeflatedBytesReadPosition,
            b, off, rval);
         
         // update read position and valid bytes
         mDeflatedBytesReadPosition += rval;
         mValidDeflatedBytes -= rval;
      }
      else
      {
         // keep reading until len is reached or the deflater cannot be filled
         while(rval < len && fillDeflater())
         {
            // since deflater was filled, data should be available
            if(rval == -1)
            {
               rval = 0;
            }
            
            // deflate data into deflated bytes buffer
            int count = mDeflater.deflate(mDeflatedBytes);
            
            // set the deflated bytes read position and valid byte count
            mDeflatedBytesReadPosition = 0;
            mValidDeflatedBytes = count;
            
            // copy data into passed read buffer
            int copySize = Math.min(len - rval, count);
            System.arraycopy(mDeflatedBytes, 0, b, off, copySize);
            
            // increment rval
            rval += copySize;
         
            // update deflated bytes read position, valid byte count
            mDeflatedBytesReadPosition += copySize;
            mValidDeflatedBytes -= copySize;
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
      mDeflater.finish();
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
      
      // end the deflater
      mDeflater.end();
   }
   
   /**
    * Gets the total number of bytes read from the underlying input stream.
    * 
    * @return the total number of bytes read from the underlying input stream.
    */
   public long getTotalUnderlyingBytesRead()
   {
      return mUnderlyingBytesRead;
   }
}
