/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

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
 * The process for decoding "chunked" transfer-coding is as follows:
 * 
 * length := 0
 * 
 * read chunk-size, chunk-extension (if any) and CRLF
 * while(chunk-size > 0)
 * {
 *    read chunk-data and CRLF
 *    append chunk-data to entity-body
 *    length := length + chunk-size
 *    read chunk-size and CRLF
 * }
 * 
 * read entity-header
 * while(entity-header not empty)
 * {
 *    append entity-header to existing header fields
 *    read entity-header
 * }
 * 
 * Content-Length := length
 * Remove "chunked" from Transfer-Encoding 
 * 
 * Information from:
 * http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
 * http://www.w3.org/Protocols/rfc2616/rfc2616-sec19.html#sec19.4.5
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
    * @return the total number of content bytes written.
    * 
    * @throws IOException
    * @throws InterruptedException
    */
   protected long writeHttpBodyImpl(
      HttpHeader header, InputStream bodyStream, HttpWebConnection hwc)
   throws IOException, InterruptedException
   {
      long rval = 0;
      
      // create a new empty chunk
      Chunk chunk = new Chunk();
      
      // keep writing chunks until the last chunk is reached
      while(!chunk.isLastChunk())
      {
         // read the chunk data from the body stream and write it out
         // to the http web connection
         rval += chunk.write(header, bodyStream, hwc);
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
    * @return the total number of content bytes read.
    * 
    * @throws IOException
    * @throws InterruptedException
    */
   protected long readHttpBodyImpl(
      HttpHeader header, HttpWebConnection hwc, OutputStream os)
   throws IOException, InterruptedException
   {
      long rval = 0;
      
      // create a new empty chunk
      Chunk chunk = new Chunk();
      
      // keep reading chunks until the last chunk is reached
      while(!chunk.isLastChunk())
      {
         // read the chunk from the http web connection and
         // write it out to the output stream
         rval += chunk.read(header, hwc, os);
      }
      
      // update content length header
      header.setContentLength(rval);
      
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
         // data length is 0
         mDataLength = 0;
         
         // chunk is not a last chunk
         mLastChunk = false;
         
         // create the IO buffer
         mIOBuffer = new byte[16384];
      }
      
      /**
       * Reads this chunk from the passed http web connection.
       * 
       * @param header the http header to update with chunk trailer headers.
       * @param hwc the http web connection to read this chunk from.
       * @param os the output stream to write the chunk data to.
       * 
       * @return the number of body bytes read.
       * 
       * @throws IOException
       * @throws InterruptedException
       */
      public int read(HttpHeader header, HttpWebConnection hwc, OutputStream os)
      throws IOException, InterruptedException
      {
         int rval = 0;
         
         // read chunk-size
         String chunkSize = hwc.readCRLF();
         
         if(chunkSize != null)
         {
            // ignore chunk-extension
            chunkSize = chunkSize.split(" ")[0];
            
            // get length of chunk data
            int length = Integer.parseInt(chunkSize, 16);
            
            // this is the last chunk if length is 0
            mLastChunk = (length == 0);
            
            // read the chunk and write it out to the passed output stream
            int numBytes = 0;
            while(length > 0 && numBytes != -1)
            {
               // get the read size
               int readSize = Math.min(length, mIOBuffer.length);
               
               // read from the web connection
               numBytes = hwc.read(mIOBuffer, 0, readSize);
               if(numBytes != -1)
               {
                  // write out chunk data
                  os.write(mIOBuffer, 0, numBytes);
                  
                  // decrement length
                  length -= numBytes;
                  
                  // increment body bytes read
                  rval += numBytes;
               }
               else
               {
                  throw new IOException("Could not read chunk!");
               }
            }
            
            // if this is the last chunk, then read in the
            // chunk trailer and last CRLF
            if(isLastChunk())
            {
               // build trailer headers
               StringBuffer trailerHeaders = new StringBuffer();
               String line = null;
               while((line = hwc.readCRLF()) != null && !line.equals(""))
               {
                  trailerHeaders.append(line + HttpHeader.CRLF);
               }
               
               // parse trailer headers
               header.parseHeaders(trailerHeaders.toString());
               
               // remove "chunked" from transfer-encoding header
               String transferEncoding = header.getTransferEncoding();
               transferEncoding.replaceAll("chunked", "");
               if(transferEncoding.equals(""))
               {
                  header.setTransferEncoding(null);
               }
               else
               {
                  header.setTransferEncoding(transferEncoding);
               }
            }
            else
            {
               // read chunk-data CRLF
               hwc.readCRLF();
            }
         }
         
         return rval;
      }
      
      /**
       * Writes a chunk to the passed output stream.
       * 
       * @param header the http header with trailer headers to use.
       * @param is the input stream to read this chunk's data from.
       * @param hwc the http web connection to write this chunk to.
       * 
       * @return the number of body bytes written.
       * 
       * @throws IOException
       * @throws InterruptedException
       */
      public int write(HttpHeader header, InputStream is, HttpWebConnection hwc)
      throws IOException, InterruptedException
      {
         int rval = 0;
         
         // get crlf bytes
         byte[] crlfBytes = HttpHeader.CRLF.getBytes();
         
         // read from the input stream
         int numBytes = 0;
         if((numBytes = is.read(mIOBuffer)) != -1)
         {
            // get the chunk-size
            String chunkSize = Integer.toHexString(numBytes);
            
            // write chunk-size
            byte[] b = chunkSize.getBytes();
            hwc.write(b, 0, b.length);
            
            // write CRLF
            hwc.write(crlfBytes, 0, crlfBytes.length);
            
            // write chunk data
            hwc.write(mIOBuffer, 0, numBytes);
            
            // increment body bytes written
            rval += numBytes;
            
            // write CRLF
            hwc.write(crlfBytes, 0, crlfBytes.length);
         }
         else
         {
            // end of stream reached, this is the last chunk
            mLastChunk = true;
            
            // write chunk-size of "0"
            byte[] b = new String("0").getBytes();
            hwc.write(b, 0, b.length);
            
            // write CRLF
            hwc.write(crlfBytes, 0, crlfBytes.length);
            
            // send content-length header
            String contentLength =
               "Content-Length: " + header.getContentLength() + HttpHeader.CRLF;
            b = contentLength.getBytes();
            hwc.write(b, 0, b.length);
            
            // write out last CRLF
            hwc.write(crlfBytes, 0, crlfBytes.length);
         }
            
         return rval;
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
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
