/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import com.db.common.stream.PipeInputStream;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

/**
 * A GZipHttpContentCoder is a class that is used to encode and decode
 * http message body content for http web requests and and http web responses
 * that have a content-encoding header value set to "gzip".  
 * 
 * @author Dave Longley
 */
public class GZipHttpContentCoder
implements HttpContentEncoder, HttpContentDecoder
{
   /**
    * Creates a new GZipHttpContentCoder object.
    */
   public GZipHttpContentCoder()
   {
   }
   
   /**
    * Checks the content-encoding for "gzip" encoding.
    * 
    * @param contentEncoding the content-encoding to check.
    * @return true if "gzip" content-encoding is used.
    */
   protected boolean isGZipContentEncoding(String contentEncoding)
   {
      boolean rval = false;
      
      rval = contentEncoding.contains("gzip");
      
      return rval;
   }
   
   /**
    * Gets an input stream to read encoded data from.
    * 
    * @param contentEncoding the content encoding for the stream.
    * @param decodedStream the input stream with the data to encode.
    * @return the input stream to read encoded data with.
    * @throws IOException
    */
   public InputStream getHttpContentEncodedStream(
      String contentEncoding, InputStream decodedStream)
   throws IOException
   {
      InputStream rval = decodedStream;
      
      if(isGZipContentEncoding(contentEncoding))
      {
         // create a pipe input stream that reads from the passed stream
         PipeInputStream pipe = new PipeInputStream(decodedStream);
         
         // wrap the output stream that writes to the pipe
         GZIPOutputStream gzos = new GZIPOutputStream(pipe.getOutputStream());
         
         // attach the output stream to the pipe
         pipe.setOutputStream(gzos);
         
         // set pipe as input stream
         rval = pipe;
      }
      
      return rval;
   }
   
   /**
    * Encodes the passed string of http content.
    * 
    * @param str the string to encode.
    * @return the encoded string.
    * @throws IOException
    */
   public String encodeHttpContentString(String str) throws IOException
   {
      byte[] data = encodeHttpContentData(str.getBytes());
      return new String(data);
   }
   
   /**
    * Encodes the passed byte array of http content.
    * 
    * @param data the byte array to encode.
    * @return the encoded byte array.
    * @throws IOException
    */
   public byte[] encodeHttpContentData(byte[] data) throws IOException
   {
      // get byte array input stream
      ByteArrayInputStream bias = new ByteArrayInputStream(data);
      
      // get encoded stream
      InputStream is = getHttpContentEncodedStream("gzip", bias);
      
      // get byte array output stream
      ByteArrayOutputStream baos = new ByteArrayOutputStream();
      
      // read in decoded data in and write out encoded data
      byte[] buffer = new byte[65536];
      int numBytes = -1;
      while((numBytes = is.read(buffer)) != -1)
      {
         baos.write(buffer, 0, numBytes);
         baos.flush();
      }
      
      // close input stream
      is.close();
      
      return baos.toByteArray();
   }
   
   /**
    * Gets an input stream to read decoded data from. This input stream must
    * not read more data than it needs because the current design filters
    * input from an http web connection directly into this stream.
    * 
    * @param contentEncoding the content encoding for the stream.
    * @param encodedStream the input stream with the data to decode.
    * @return the input stream to read decoded data from.
    * @throws IOException
    */
   public InputStream getHttpContentDecodedStream(
      String contentEncoding, InputStream encodedStream)
   throws IOException
   {
      InputStream rval = encodedStream;
      
      if(isGZipContentEncoding(contentEncoding))
      {
         // wrap the input stream with a gzip input stream
         GZIPInputStream gzis = new GZIPInputStream(encodedStream);
         
         // set gzip stream as input stream
         rval = gzis;
      }
      
      return rval;
   }
   
   /**
    * Decodes the passed string of http content.
    * 
    * @param str the string to decode.
    * @return the decoded string.
    * @throws IOException
    */
   public String decodeHttpContentString(String str) throws IOException
   {
      byte[] data = decodeHttpContentData(str.getBytes());
      return new String(data);
   }
   
   /**
    * Decodes the passed byte array of http content.
    * 
    * @param data the byte array to decode.
    * @return the decoded byte array.
    * @throws IOException
    */
   public byte[] decodeHttpContentData(byte[] data) throws IOException
   {
      // get byte array input stream
      ByteArrayInputStream bias = new ByteArrayInputStream(data);
      
      // get decoded stream
      InputStream is = getHttpContentDecodedStream("gzip", bias);

      // get byte array output stream
      ByteArrayOutputStream baos = new ByteArrayOutputStream();
      
      // read in encoded data and write out decoded data
      byte[] buffer = new byte[65536];
      int numBytes = -1;
      while((numBytes = is.read(buffer)) != -1)
      {
         baos.write(buffer, 0, numBytes);
         baos.flush();
      }
      
      // close input stream
      is.close();
      
      return baos.toByteArray();      
   }
}
