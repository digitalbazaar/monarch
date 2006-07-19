/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.zip.DataFormatException;
import java.util.zip.Inflater;

/**
 * An InflaterOutputStream is used to "inflate" (decompress) data as it
 * is written.
 * 
 * @author Dave Longley
 */
public class InflaterOutputStream extends FilterOutputStream
{
   /**
    * The inflater (decompressor) for this stream.
    */
   protected Inflater mInflater;
   
   /**
    * A buffer used to store deflated (compressed) bytes that need
    * to be inflated (decompressed).
    */
   protected byte[] mDeflatedBytes;
   
   /**
    * The number of valid bytes in the deflated bytes buffer.
    */
   protected int mValidDeflatedBytes;
   
   /**
    * A buffer used to store inflated (decompressed) bytes for writing.
    */
   protected byte[] mInflatedBytes;
   
   /**
    * A single byte buffer to use when writing one byte via write().
    */
   protected byte[] mSingleByteBuffer;
   
   /**
    * Creates a new InflaterOutputStream with a default Inflater and
    * default buffer size of 2048 bytes.
    * 
    * @param os the underlying output stream to write to.
    */
   public InflaterOutputStream(OutputStream os)
   {
      this(os, new Inflater(), 2048);
   }
   
   /**
    * Creates a new InflaterOutputStream with a default buffer size of
    * 2048 bytes.
    * 
    * @param os the underlying output stream to write to.
    * @param inflater the inflater to use.
    */
   public InflaterOutputStream(OutputStream os, Inflater inflater)
   {
      this(os, inflater, 2048);
   }
   
   /**
    * Creates a new InflaterOutputStream.
    * 
    * @param os the underlying output stream to write to.
    * @param inflater the inflater to use.
    * @param bufferSize the size of the internal write buffer.
    */
   public InflaterOutputStream(
      OutputStream os, Inflater inflater, int bufferSize)
   {
      // store underlying output stream
      super(os);
      
      // throw exception is buffer size is <= 0
      if(bufferSize <= 0)
      {
         throw new IllegalArgumentException("bufferSize must be > 0");
      }
      
      // store inflater
      mInflater = inflater;
      
      // create the internal write buffer for deflated bytes
      mDeflatedBytes = new byte[2048];

      // valid bytes is 0
      mValidDeflatedBytes = 0;
      
      // create the inflated bytes buffer
      mInflatedBytes = new byte[bufferSize];
      
      // create the single byte buffer
      mSingleByteBuffer = new byte[1];
   }
   
   /**
    * Stores the passed deflated (compressed) bytes in the internal 
    * deflated bytes buffer.
    * 
    * @param buffer the array of deflated bytes.
    * @param offset the offset of the deflated bytes.
    * @param length the number of valid deflated bytes in the array.
    */
   protected void storeDeflatedBytes(byte[] buffer, int offset, int length)
   {
      // resize the internal buffer as necessary
      int size = mValidDeflatedBytes + length;
      if(size > mDeflatedBytes.length)
      {
         size = Math.max(size, mDeflatedBytes.length * 2);
         
         byte[] newBuffer = new byte[size];
         System.arraycopy(mDeflatedBytes, 0, newBuffer, 0, mValidDeflatedBytes);
         mDeflatedBytes = newBuffer;
      }
      
      // copy passed bytes into deflated bytes buffer
      System.arraycopy(buffer, offset,
         mDeflatedBytes, mValidDeflatedBytes, length);
      mValidDeflatedBytes += length;
   }
   
   /**
    * Writes all available inflated data out to the underlying output stream.
    * 
    * @throws IOException
    */
   protected void writeInflatedBytes() throws IOException
   {
      try
      {
         // write the remaining data out
         while(!getInflater().finished() &&
               !getInflater().needsInput() &&
               !getInflater().needsDictionary())
         {
            // deflated bytes are no longer valid
            mValidDeflatedBytes = 0;
            
            // inflate bytes
            int numBytes = getInflater().inflate(mInflatedBytes);
            
            // write to underlying output stream
            out.write(mInflatedBytes, 0, numBytes);
         }
      }
      catch(DataFormatException e)
      {
         throw new IOException("" + e);
      }
   }
   
   /**
    * Inflates (decompresses) the passed deflated (compressed) byte and
    * writes it to the output stream.
    * 
    * @param b the byte to inflate and write.
    * 
    * @throws IOException
    */
   public void write(int b) throws IOException
   {
      mSingleByteBuffer[0] = (byte)(b & 0xff);
      write(mSingleByteBuffer);
   }

   /**
    * Inflates (decompresses) the passed array of deflated (compressed)
    * bytes and writes them to the output stream.
    * 
    * @param b the deflated data to inflate and write.
    * @param off the start offset of the data.
    * @param len the length of the data.
    * 
    * @throws IOException
    */
   public void write(byte[] b, int off, int len)
   throws IOException
   {
      if(!getInflater().finished())
      {
         // store passed deflated bytes
         storeDeflatedBytes(b, off, len);
         
         // use the total stored bytes as the inflater data
         getInflater().setInput(mDeflatedBytes, 0, mValidDeflatedBytes);
         
         // write out any inflated bytes
         writeInflatedBytes();
      }
   }
   
   /**
    * Writes any remaining inflated (uncompressed) data and DOES NOT
    * close the underlying input stream.
    * 
    * This method should be used if the underlying output stream is
    * in a chain that should not be broken/closed. 
    * 
    * @throws IOException
    */
   public void finish() throws IOException
   {
      writeInflatedBytes();      
   }
   
   /**
    * Writes any remaining inflated (uncompressed) data and closes the
    * underlying input stream.
    * 
    * @throws IOException
    */
   public void close() throws IOException
   {
      // finish
      finish();
      
      // close underlying output stream
      out.close();
   }

   /**
    * Gets the inflater used by this output stream.
    * 
    * @return the inflater used by this output stream.
    */
   public Inflater getInflater()
   {
      return mInflater;
   }
}
