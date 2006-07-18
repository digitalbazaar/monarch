/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.IOException;
import java.io.OutputStream;
import java.util.zip.CRC32;
import java.util.zip.Inflater;

import com.db.data.format.GZipHeader;
import com.db.data.format.GZipTrailer;

/**
 * An UnGZipOutputStream un-gzips data and writes it to an output stream.
 * 
 * @author Dave Longley
 */
public class UnGZipOutputStream extends InflaterOutputStream
{
   /**
    * A CRC-32 for computing the CRC-32 of the streaming uncompressed data.
    */
   protected CRC32 mCrc32;
   
   /**
    * The GZipHeader for converting the gzip header.
    */
   protected GZipHeader mHeader;
   
   /**
    * The GZipTrailer for converting the gzip trailer.
    */
   protected GZipTrailer mTrailer;
   
   /**
    * For storing the header for the gzipped data.
    */
   protected byte[] mHeaderBuffer;
   
   /**
    * The number of header bytes in the header buffer so far.
    */
   protected int mHeaderBytes;
   
   /**
    * The required number of header bytes.
    */
   protected int mRequiredHeaderBytes;
   
   /**
    * For storing the trailer for the gzipped data.
    */
   protected byte[] mTrailerBuffer;
   
   /**
    * The number of trailer bytes in the trailer buffer so far.
    */
   protected int mTrailerBytes;
   
   /**
    * The required number of trailer bytes.
    */
   protected int mRequiredTrailerBytes;
   
   /**
    * Set to true once the header has been read.
    */
   protected boolean mHeaderRead;
   
   /**
    * Creates a new UnGZipOutputStream.
    * 
    * @param os the output stream to write uncompressed data to.
    */
   public UnGZipOutputStream(OutputStream os)
   {
      // create an inflater that supports GZIP
      super(os, new Inflater(true));
      
      // create a new CRC-32 for calculating the CRC-32 of the
      // uncompressed data 
      mCrc32 = new CRC32();
      
      // create a new gzip header
      mHeader = new GZipHeader();
      
      // create a new gzip trailer
      mTrailer = new GZipTrailer();
      
      // create the header buffer
      mHeaderBuffer = new byte[10];
      
      // no header bytes yet
      mHeaderBytes = 0;
      
      // required header bytes so far
      mRequiredHeaderBytes = 10;
      
      // create the trailer buffer
      mTrailerBuffer = new byte[8];
      
      // no trailer bytes yet
      mTrailerBytes = 0;
      
      // required trailer bytes so far
      mRequiredTrailerBytes = 8;
      
      // header not read yet
      mHeaderRead = false;
   }
   
   /**
    * Stores the passed header bytes in the internal header buffer.
    * 
    * @param buffer the array of header bytes.
    * @param offset the offset of the header bytes.
    * @param length the number of header bytes in the array.
    */
   protected void storeHeaderBytes(byte[] buffer, int offset, int length)
   {
      // resize the internal buffer as necessary
      int size = mHeaderBytes + mRequiredHeaderBytes;
      
      if(mHeaderBytes + mRequiredHeaderBytes < mHeaderBuffer.length)
      {
         size = Math.max(size, mHeaderBuffer.length * 2);
         
         byte[] newBuffer = new byte[size];
         System.arraycopy(mHeaderBuffer, 0, newBuffer, 0, mHeaderBytes);
         mHeaderBuffer = newBuffer;
      }
      
      // get the copy size
      int copySize = Math.min(mRequiredHeaderBytes, length);
      
      // copy passed bytes into header bytes buffer
      System.arraycopy(buffer, offset, mHeaderBuffer, mHeaderBytes, copySize);
      
      // update required header bytes
      mRequiredHeaderBytes -= copySize;
   }
   
   /**
    * Stores the passed trailer bytes in the internal trailer buffer.
    * 
    * @param buffer the array of trailer bytes.
    * @param offset the offset of the trailer bytes.
    * @param length the number of trailer bytes in the array.
    */
   protected void storeTrailerBytes(byte[] buffer, int offset, int length)
   {
      // get the copy size
      int copySize = Math.min(mRequiredTrailerBytes, length);
      
      // copy passed bytes into trailer bytes buffer
      System.arraycopy(buffer, offset, mTrailerBuffer, mTrailerBytes, copySize);
      
      // update required trailer bytes
      mRequiredTrailerBytes -= copySize;
   }
   
   /**
    * Tries to read the gzip header. Updates the number of required header
    * bytes.
    *
    * @param b the gzipped data.
    * @param off the start offset of the data.
    * @param len the length of the data.
    */
   protected void readHeader(byte[] b, int off, int len) throws IOException
   {
      // store the header bytes
      storeHeaderBytes(b, off, len);
      
      // try to convert the header
      mHeader.convertFromBytes(mHeaderBuffer, 0, mHeaderBytes);
   }
   
   /**
    * Un-gzipped the passed array of gzipped bytes and writes them to the
    * output stream.
    * 
    * @param b the gzipped data to un-gzip and write.
    * @param off the start offset of the data.
    * @param len the length of the data.
    * 
    * @throws IOException
    */
   public void write(byte[] b, int off, int len) throws IOException
   {
      // write out data if the inflater isn't finished
      if(!getInflater().finished())
      {
         // try to read the header if it hasn't been read yet
         if(!mHeaderRead)
         {
            // get the number of bytes that will exceed the required header
            // bytes
            int extraBytes = len - mRequiredHeaderBytes;
            
            // read the header
            readHeader(b, off, len);
           
            // see if there are extra bytes
            if(extraBytes > 0)
            {
               // header has been read, but there may be data to write out
               mHeaderRead = true;
               
               // write out data that isn't in the header
               super.write(b, off + len - extraBytes, extraBytes);
            }
            else if(extraBytes == 0)
            {
               // read exactly the right number of bytes, nothing to write out
               mHeaderRead = true;
            }
            
            if(mHeaderRead)
            {
               // throw out the header buffer, it is no longer needed
               mHeaderBuffer = null;
            }
         }
         else
         {
            // write out inflated data
            super.write(b, off, len);
         }
      }
      else
      {
         // read in the trailer
         
      }
   }
}
