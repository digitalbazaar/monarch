/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

/**
 * A TruncateFileInputStream is used to read a file in an truncate it at
 * a certain length.
 * 
 * @author Dave Longley
 */
public class TruncateFileInputStream extends FileInputStream
{
   /**
    * The truncate length.
    */
   protected long mTruncateLength;
   
   /**
    * Stores the number of bytes remaining.
    */
   protected long mBytesRemaining;
   
   /**
    * A simple read buffer for doing quick read()s.
    */
   protected byte[] mOneByteBuffer;
   
   /**
    * Creates a new TruncateFileInputStream that reads from the passed
    * file and truncates it at the specified length.
    * 
    * @param file the File to read from.
    * @param length the length to truncate the file at.
    * 
    * @exception FileNotFoundException thrown if the file does not exist.
    */
   public TruncateFileInputStream(File file, long length)
   throws FileNotFoundException
   {
      super(file);

      // store truncate length
      mTruncateLength = length;
      
      // bytes remaining at max
      mBytesRemaining = length;
      
      // create one byte buffer
      mOneByteBuffer = new byte[1];
   }
   
   /**
    * Reads a single byte and returns it. If the end of the stream has been
    * reached, then -1 will be returned. This method will block until a
    * byte can be read or until the end of the stream is reached.
    *  
    * @return the byte read or -1 if end of the stream was reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   @Override
   public int read() throws IOException
   {
      int rval = -1;
      
      // read into the one byte buffer
      if(read(mOneByteBuffer, 0, 1) != -1)
      {
         rval = mOneByteBuffer[0] & 0xff;
      }
      
      return rval;
   }
   
   /**
    * Reads up to length bytes of data from this input stream. The number of
    * bytes read will be returned unless the end of the stream is reached,
    * in which case -1 will be returned. This method will block until at least
    * one byte can be read or until the end of the stream is reached.
    * 
    * @param b the data buffer to read data into.
    * @param offset the offset at which to store the read data.
    * @param length the maximum number of bytes to read.
    * 
    * @return the number of bytes read or -1 if the end of the stream was
    *         reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   @Override
   public int read(byte[] b, int offset, int length) throws IOException
   {
      int rval = -1;
      
      if(mBytesRemaining > 0)
      {
         rval = super.read(b, offset, Math.min((int)mBytesRemaining, length));
         if(rval != -1)
         {
            mBytesRemaining -= rval;
         }
      }
      
      return rval;
   }
   
   /**
    * Skips up to the passed number of bytes. The actual number of bytes
    * skipped will be returned. This method will block until at least one
    * byte can be skipped or until the end of the stream is reached.
    *
    * @param n the number of bytes to be skipped.
    * 
    * @return the actual number of bytes skipped.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   @Override
   public long skip(long n) throws IOException
   {
      long rval = 0;
      
      long remaining = n;
      
      // read into dummy buffer
      byte[] b = new byte[2048];
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
}
