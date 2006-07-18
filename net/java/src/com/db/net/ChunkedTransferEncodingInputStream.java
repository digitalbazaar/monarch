/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * A ChunkedTransferEncodingInputStream is an input stream that can
 * read either chunked transfer-encoded http content and convert it
 * to unchunked content, or unchunked content and convert it to
 * transfer-encoded http content.
 * 
 * @author Dave Longley
 */
public class ChunkedTransferEncodingInputStream extends FilterInputStream
{
   /**
    * True if this stream should encode data as it is read, false if
    * it should decode it.
    */
   protected boolean mEncode;
   
   /**
    * The encode buffer.
    */
   protected byte[] mEncodeBuffer;
   
   /**
    * Creates a new ChunkedTransferEncodingInputStream.
    * 
    * @param is the input stream to read the data from.
    * @param encode true to encode the data that is read, false to decode it.
    */
   public ChunkedTransferEncodingInputStream(InputStream is, boolean encode)
   {
      super(is);
      
      // store encode mode
      mEncode = encode;
   }
   
   /**
    * Gets whether the read data should be encoded or decoded.
    *
    * @return if encoding should be used, false if decoding should be used.
    */
   protected boolean useEncoding()
   {
      return mEncode;
   }
   
   /**
    * Reads the next byte of data from this input stream. The value 
    * byte is returned as an <code>int</code> in the range 
    * <code>0</code> to <code>255</code>. If no byte is available 
    * because the end of the stream has been reached, the value 
    * <code>-1</code> is returned. This method blocks until input data 
    * is available, the end of the stream is detected, or an exception 
    * is thrown.
    *
    * @return the next byte of data, or <code>-1</code> if the end of the
    *         stream is reached.
    *         
    * @exception IOException if an I/O error occurs.
    */
   public int read() throws IOException
   {
      int rval = -1;
      
      // handle encoding
      if(useEncoding())
      {
         // reading one byte will always return '0'
         rval = '0';
      }
      else
      {
         // handle decoding
         // FIXME:
      }
      
      return rval;
   }

   /**
    * Reads up to <code>byte.length</code> bytes of data from this 
    * input stream into an array of bytes. This method blocks until some 
    * input is available. 
    * <p>
    * This method simply performs the call
    * <code>read(b, 0, b.length)</code> and returns
    * the  result. It is important that it does
    * <i>not</i> do <code>in.read(b)</code> instead;
    * certain subclasses of  <code>FilterInputStream</code>
    * depend on the implementation strategy actually
    * used.
    *
    * @param b the buffer into which the data is read.
    * 
    * @return the total number of bytes read into the buffer, or
    *         <code>-1</code> if there is no more data because the end of
    *         the stream has been reached.
    *         
    * @exception IOException if an I/O error occurs.
    */
   public int read(byte b[]) throws IOException
   {
      return read(b, 0, b.length);
   }

   /**
    * Reads up to <code>len</code> bytes of data from this input stream 
    * into an array of bytes. This method blocks until some input is 
    * available. 
    * <p>
    * This method simply performs <code>in.read(b, off, len)</code> 
    * and returns the result.
    *
    * @param b the buffer into which the data is read.
    * @param off the start offset of the data.
    * @param len the maximum number of bytes read.
    * 
    * @return the total number of bytes read into the buffer, or
    *         <code>-1</code> if there is no more data because the end of
    *         the stream has been reached.
    *         
    * @exception IOException if an I/O error occurs.
    */
   public int read(byte b[], int off, int len) throws IOException
   {
      int numBytes = -1;
      
      // the length requested is 0, return 0
      if(len == 0)
      {
         numBytes = 0;
      }
      else
      {
         // handle encoding
         if(useEncoding())
         {
            // get the number of bytes that can be read without blocking
            int readable = in.available();
            if(readable > 0)
            {
               // if the length requested is less than 3, then return nothing
               if(len < 3)
               {
                  numBytes = 0;
               }
               else
               {
                  // FIXME:
                  // read bytes from underlying stream
                  //if(readable)
               }
            }
         }
         else
         {
            // handle decoding
            // FIXME:
         }
      }
      
      return numBytes;
   }
}
