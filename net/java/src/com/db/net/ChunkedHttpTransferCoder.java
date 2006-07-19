/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * A ChunkedHttpTransferCoder is a class that is used to encode and decode
 * http message body transfers for http web requests and and http web responses
 * that have a transfer-encoding header value set to "chunked".
 * 
 * Chunked Transfer Coding breaks an http message into a series of chunks,
 * each with its own size indicator and an optional trailer containing
 * entity-header fields.
 * 
 * The format is as follows:
 * 
 * Chunked-Body =
 * chunk
 * last-chunk
 * trailer
 * CRLF
 * 
 * chunk =
 * chunk-size [chunk-extension] CRLF
 * chunk-data CRLF
 * 
 * chunk-size = 1*HEX
 * last-chunk = 1*("0") [chunk-extension] CRLF
 * 
 * chunk-extension =
 * ( ";" chunk-ext-name [ "=" chunk-ext-val ] )
 * 
 * chunk-ext-name = token
 * chunk-ext-val = token | quoted-string
 * chunk-data = chunk-size(OCTET)
 * 
 * trailer = *(entity-header CRLF)
 * 
 * Information from: http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
 * 
 * @author Dave Longley
 */
public class ChunkedHttpTransferCoder extends AbstractHttpTransferCoder
{
   /**
    * Creates a new ChunkedHttpTransferCoder object.
    */
   public ChunkedHttpTransferCoder()
   {
      super("chunked");
   }
   
   /**
    * Reads data from the passed input stream until the end of the stream,
    * encodes the data according to the supported transfer-encoding, and
    * then writes the data out to the passed http web connection.
    * 
    * @param header the http header for the http body.
    * @param bodyStream the input stream with the body to encode.
    * @param hwc the http web connection to write the http body to.
    * 
    * @return the total number of bytes written.
    * 
    * @throws IOException
    */
   protected long writeHttpBodyImpl(
      HttpHeader header, InputStream bodyStream, HttpWebConnection hwc)
   throws IOException
   {
      long rval = 0;
      
      // create a new empty chunk
      Chunk chunk = new Chunk();
      
      // keep writing chunks until the last chunk is reached
      while(!chunk.isLastChunk())
      {
         // read the chunk data from the body stream
         chunk.readData(bodyStream);
      
         // write the chunk to the http web connection
         rval += chunk.write(hwc);
      }
      
      return rval;
   }
   
   /**
    * Reads and decodes an http message body from the http web connection,
    * according to the supported transfer-encoding, and writes it to the
    * passed output stream, unless the output stream is null.
    *  
    * @param header the http header for the http body.
    * @param hwc the http web connection to read the http body from. 
    * @param os the output stream to write the body to.
    * 
    * @return the total number of bytes read.
    * 
    * @throws IOException
    */
   protected long readHttpBodyImpl(
      HttpHeader header, HttpWebConnection hwc, OutputStream os)
   throws IOException
   {
      long rval = 0;
      
      // create a new empty chunk
      Chunk chunk = new Chunk();
      
      // keep reading chunks until the last chunk is reached
      while(!chunk.isLastChunk())
      {
         // read the chunk from the http web connection
         rval += chunk.read(hwc);
      
         // write the chunk's data to the output stream
         chunk.writeData(os);
      }
      
      return rval;
   }
   
   /**
    * A Chunk represents an http Chunk according to the Chunked Transfer
    * Coding.
    * 
    * It has the following format:
    * 
    * chunk =
    * chunk-size [chunk-extension] CRLF
    * chunk-data CRLF
    * 
    * chunk-size = 1*HEX
    * last-chunk = 1*("0") [chunk-extension] CRLF
    * 
    * chunk-extension =
    * ( ";" chunk-ext-name [ "=" chunk-ext-val ] )
    * 
    * chunk-ext-name = token
    * chunk-ext-val = token | quoted-string
    * chunk-data = chunk-size(OCTET)
    * 
    * Information from: http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    * 
    * @author Dave Longley
    */
   public class Chunk
   {
      /**
       * The byte array with the data for this chunk.
       */
      protected byte[] mDataBuffer;
      
      /**
       * The offset in the byte array for the data for this chunk.
       */
      protected int mDataOffset;
      
      /**
       * The length of the data for this chunk.
       */
      protected int mDataLength;
      
      /**
       * Set to true if this chunk is a last chunk, false if not.
       */
      protected boolean mLastChunk;
      
      /**
       * A read/write buffer for this chunk.
       */
      protected byte[] mIOBuffer;
      
      /**
       * Creates a new empty Chunk.
       */
      public Chunk()
      {
         mDataBuffer = new byte[0];
         mDataOffset = 0;
         mDataLength = 0;
         
         // chunk is not a last chunk
         mLastChunk = false;
         
         // create the IO buffer
         mIOBuffer = new byte[16384];
      }
      
      /**
       * Reads this chunk from the passed http web connection.
       * 
       * @param hwc the http web connection to read this chunk from.
       * 
       * @return the number of bytes read.
       * 
       * @throws IOException
       */
      public int read(HttpWebConnection hwc)
      {
         int rval = 0;
         
         // FIXME:
         
         return rval;
      }
      
      /**
       * Reads this chunk's data from the passed input stream.
       * 
       * @param is the input stream to read this chunk's data from.
       * 
       * @throws IOException
       */
      public void readData(InputStream is) 
      {
         // FIXME:
      }
      
      /**
       * Writes this chunk to the passed output stream.
       * 
       * @param hwc the http web connection to write this chunk to.
       * 
       * @return the number of bytes written.
       * 
       * @throws IOException
       */
      public int write(HttpWebConnection hwc)
      {
         int rval = 0;
         
         // FIXME:
         
         return rval;
      }
      
      /**
       * Writes this chunk's data to the passed output stream.
       * 
       * @param os the output stream to write this chunks data to.
       * 
       * @throws IOException
       */
      public void writeData(OutputStream os)
      {
         // FIXME:
      }
      
      /**
       * Returns true if this chunk is a last chunk.
       * 
       * @return true if this chunk is a last chunk, false if not.
       */
      public boolean isLastChunk()
      {
         return mLastChunk;
      }
   }
}
