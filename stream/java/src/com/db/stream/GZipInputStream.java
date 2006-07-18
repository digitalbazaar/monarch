/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.zip.CRC32;
import java.util.zip.Deflater;

import com.db.data.format.GZipHeader;
import com.db.data.format.GZipTrailer;

/**
 * A GZipInputStream reads uncompressed data and gzips it.
 * 
 * @author Dave Longley
 */
public class GZipInputStream extends DeflaterInputStream
{
   /**
    * A CRC-32 for calculating the CRC-32 of the uncompressed data as
    * it is read.
    */
   protected CRC32 mCrc32;
   
   /**
    * The byte array input stream for reading from the gzip header.
    */
   protected ByteArrayInputStream mHeaderInputStream;
   
   /**
    * True if the gzip header has been read already, false if not.
    */
   protected boolean mHeaderRead;
   
   /**
    * The byte array input stream for reading the gzip trailer.
    */
   protected ByteArrayInputStream mTrailerInputStream;
   
   /**
    * True if the gzip trailer has been read already, false if not.
    */
   protected boolean mTrailerRead;
   
   /**
    * Creates a new GZipInputStream using a compression level of
    * Deflater.DEFAULT_COMPRESSION level.
    * 
    * @param is the input stream to read the uncompressed data from.
    */
   public GZipInputStream(InputStream is)
   {
      // create a deflater that supports GZIP
      this(is, Deflater.DEFAULT_COMPRESSION);
   }
   
   /**
    * Creates a new GZipInputStream.
    * 
    * @param is the input stream to read the uncompressed data from.
    * @param level the Deflater compression level to use (0-9), see
    *              Deflater class.
    */
   public GZipInputStream(InputStream is, int level)
   {
      // create a deflater that supports GZIP
      super(is, new Deflater(level, true));
      
      // create the CRC-32 for calculating the CRC-32 of the uncompressed data
      mCrc32 = new CRC32();
      
      // create the header input stream
      GZipHeader header = new GZipHeader();
      header.setFHCrcFlag(true);
      mHeaderInputStream = new ByteArrayInputStream(header.convertToBytes());
      
      // the header has not yet been read
      mHeaderRead = false;
      
      // the trailer input stream will be created after the uncompressed
      // data has been compressed
      mTrailerInputStream = null;
      
      // the trailer has not yet been read
      mTrailerRead = false;
   }
   
   /**
    * Reads up to <code>len</code> bytes of data from this input stream 
    * into an array of bytes. This method blocks until some input is 
    * available.
    * 
    * The data that is read from the underlying stream is gzipped as it
    * read.
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
      
      // see if the trailer still hasn't been read
      if(!mTrailerRead)
      {
         // read from the header if it hasn't been read yet
         if(!mHeaderRead)
         {
            int readSize = Math.min(mHeaderInputStream.available(), len);
            
            // read from header input stream
            mHeaderInputStream.read(b, off, readSize);
            
            // decrease amount to read and increase offset
            len -= readSize;
            off += readSize;
            
            // set bytes read so far
            rval = readSize;
            
            // see if the header has been read
            if(mHeaderInputStream.available() == 0)
            {
               mHeaderRead = true;
            }
         }
         
         // see if the header has been read
         if(mHeaderRead)
         {
            // read from the deflater input stream super class if appropriate
            if(!getDeflater().finished())
            {
                rval += super.read(b, off, len);
            }
            else
            {
               // create the trailer input stream if it isn't already created
               if(mTrailerInputStream == null)
               {
                  // create trailer, set CRC-32 and ISIZE
                  GZipTrailer trailer = new GZipTrailer();
                  trailer.setCrc32Value(mCrc32.getValue());
                  trailer.setISize(getDeflater().getBytesRead());
                  
                  mTrailerInputStream = new ByteArrayInputStream(
                     trailer.convertToBytes());
               }
               
               // read from the trailer input stream
               int numBytes = mTrailerInputStream.read(b, off, len);
               if(numBytes != -1)
               {
                  rval += numBytes;
               }
               else
               {
                  // trailer has been read
                  mTrailerRead = true;
               }
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
      
      // return 1 if the trailer hasn't been read yet
      if(!mTrailerRead)
      {
         rval = 1;
      }
      
      return rval;
   }   
}
