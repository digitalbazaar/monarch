/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.IOException;
import java.io.OutputStream;
import java.util.zip.CRC32;
import java.util.zip.CheckedOutputStream;
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
    * Set to true once the trailer has been read.
    */
   protected boolean mTrailerRead;
   
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
      
      // trailer not read yet
      mTrailerRead = false;
   }
   
   /**
    * Stores the passed header bytes in the internal header buffer.
    * 
    * @param buffer the array of header bytes.
    * @param offset the offset of the header bytes.
    * @param length the number of header bytes in the array.
    * 
    * @return the number of bytes stored from the passed buffer.
    */
   protected int storeHeaderBytes(byte[] buffer, int offset, int length)
   {
      int rval = 0;
      
      // resize the internal buffer as necessary
      int size = mHeaderBytes + mRequiredHeaderBytes;
      
      if(size < mHeaderBuffer.length)
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
      
      // update header bytes
      mHeaderBytes += copySize;
      
      // return copy size
      rval = copySize;
      
      return rval;
   }
   
   /**
    * Tries to read the gzip header. Updates the number of required header
    * bytes. Returns the number of extra bytes in the passed buffer that
    * are not in the header.
    *
    * @param b the gzipped data.
    * @param off the start offset of the data.
    * @param len the length of the data.
    * 
    * @return the number of extra bytes in the buffer that are not in
    *         the header.
    * 
    * @throws IOException
    */
   protected int readHeader(byte[] b, int off, int len) throws IOException
   {
      int rval = 0;
      
      // if the header bytes are 0, try to convert the header from
      // the passed buffer
      if(mHeaderBytes == 0)
      {
         // get the required header bytes
         mRequiredHeaderBytes = mHeader.convertFromBytes(b, off, len);
      }
         
      // keep storing bytes until there is no more to store or the
      // header is successfully read
      while(mRequiredHeaderBytes != 0 && len > 0) 
      {
         // store the header bytes, update length and offset
         int copySize = storeHeaderBytes(b, off, len);
         len -= copySize;
         off += copySize;
         
         // try to convert the header from the internal buffer again
         mRequiredHeaderBytes = mHeader.convertFromBytes(
            mHeaderBuffer, 0, mHeaderBytes);
      }
      
      // mark header as read if required bytes == 0
      if(mRequiredHeaderBytes == 0)
      {
         mHeaderRead = true;
         
         // throw out the header buffer, it is no longer needed
         mHeaderBuffer = null;
         
         // wrap the output stream in a checked output stream
         out = new CheckedOutputStream(out, mCrc32);
      }
      
      // get the number of extra bytes
      rval = len;
      
      return rval;
   }
   
   /**
    * Stores the passed trailer bytes in the internal trailer buffer.
    * 
    * @param buffer the array of trailer bytes.
    * @param offset the offset of the trailer bytes.
    * @param length the number of trailer bytes in the array.
    * 
    * @return the number of bytes stored from the passed buffer.
    */
   protected int storeTrailerBytes(byte[] buffer, int offset, int length)
   {
      int rval = 0;
      
      // get the copy size
      int copySize = Math.min(mRequiredTrailerBytes, length);
      
      // copy passed bytes into trailer bytes buffer
      System.arraycopy(buffer, offset, mTrailerBuffer, mTrailerBytes, copySize);
      
      // update trailer bytes
      mTrailerBytes += copySize;

      // return copy size
      rval = copySize;
      
      return rval;
   }
   
   /**
    * Tries to read the gzip trailer. Updates the number of required trailer
    * bytes. Returns the number of extra bytes in the passed buffer that
    * are not in the trailer.
    *
    * @param b the gzipped data.
    * @param off the start offset of the data.
    * @param len the length of the data.
    * 
    * @return the number of extra bytes in the buffer that are not in
    *         the trailer.
    * 
    * @throws IOException
    */
   protected int readTrailer(byte[] b, int off, int len) throws IOException
   {
      int rval = 0;
      
      // if the header bytes are 0, try to convert the trailer from
      // the passed buffer
      if(mHeaderBytes == 0)
      {
         // get the required trailer bytes
         mRequiredTrailerBytes = mTrailer.convertFromBytes(b, off, len);
      }
         
      // keep storing bytes until there is no more to store or the
      // trailer is successfully read
      while(mRequiredTrailerBytes != 0 && len > 0) 
      {
         // store the trailer bytes, update length and offset
         int copySize = storeTrailerBytes(b, off, len);
         len -= copySize;
         off += copySize;
         
         // try to convert the trailer from the internal buffer again
         mRequiredTrailerBytes = mTrailer.convertFromBytes(
            mTrailerBuffer, 0, mTrailerBytes);
      }
      
      // mark trailer as read if required bytes == 0
      if(mRequiredTrailerBytes == 0)
      {
         mTrailerRead = true;
         
         // see if the trailer's CRC-32 value matches the calculated one
         
         
         // throw out the trailer buffer, it is no longer needed
         mTrailerBuffer = null;
      }
      
      // get the number of extra bytes
      rval = len;
      
      return rval;
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
            // try to read the header
            int extraBytes = readHeader(b, off, len);
            
            // see if there are extra bytes
            if(extraBytes > 0)
            {
               // write out the extra bytes that aren't in the header
               super.write(b, off + len - extraBytes, extraBytes);
            }
         }
         else
         {
            // write out inflated data
            super.write(b, off, len);
         }
      }
      else if(!mTrailerRead)
      {
         // read in the trailer
         if(readTrailer(b, off, len) > 0)
         {
            throw new IOException("GZip trailer already read!");
         }
      }
      else
      {
         throw new IOException("GZip trailer already read!");
      }
   }
}
