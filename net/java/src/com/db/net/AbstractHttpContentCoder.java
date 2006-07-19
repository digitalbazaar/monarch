/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * An AbstractHttpContentCoder is a class that is used to encode and decode
 * http message body content for http web requests and and http web responses.
 * 
 * It implements some of the basic methods for HttpContentEncoder and
 * HttpContentDecoder to allow easier extending.
 * 
 * @author Dave Longley
 */
public abstract class AbstractHttpContentCoder
implements HttpContentEncoder, HttpContentDecoder
{
   /**
    * The supported content-encoding.
    */
   protected String mSupportedContentEncoding;
   
   /**
    * Creates a new AbstractHttpContentCoder object.
    * 
    * @param supportedContentEncoding the supported content-encoding.
    */
   public AbstractHttpContentCoder(String supportedContentEncoding)
   {
      mSupportedContentEncoding = supportedContentEncoding;
   }
   
   /**
    * Checks to see if the passed content-encoding is supported.
    * 
    * @param contentEncoding the content-encoding to check.
    * 
    * @return true if the passed content-encoding is supported, false if not.
    */
   protected boolean isContentEncodingSupported(String contentEncoding)
   {
      boolean rval = false;
      
      rval = contentEncoding.startsWith(getSupportedContentEncoding());
      
      return rval;
   }
   
   /**
    * Gets an input stream to read encoded data from.
    * 
    * @param unencodedStream the input stream with the data to encode.
    * 
    * @return the input stream to read encoded data with.
    * 
    * @throws IOException
    */
   protected abstract InputStream getHttpContentEncodedStream(
      InputStream unencodedStream)
   throws IOException;
   
   /**
    * Gets an output stream to write decoded data with.
    * 
    * @param os the output stream to write the decoded data to.
    * 
    * @return the output stream to write decoded data with.
    * 
    * @throws IOException
    */
   protected abstract OutputStream getHttpContentDecodedStream(OutputStream os)
   throws IOException;
   
   /**
    * Gets an input stream to read encoded data from.
    * 
    * @param contentEncoding the content-encoding for the stream.
    * @param unencodedStream the input stream with the data to encode.
    * 
    * @return the input stream to read encoded data with.
    * 
    * @throws IOException
    */
   public InputStream getHttpContentEncodedStream(
      String contentEncoding, InputStream unencodedStream)
   throws IOException
   {
      InputStream rval = unencodedStream;
      
      // see if the content-encoding is supported
      if(isContentEncodingSupported(contentEncoding))
      {
         // get encoded stream
         rval = getHttpContentEncodedStream(unencodedStream);
      }
      
      return rval;
   }
   
   /**
    * Encodes the passed string of http content.
    * 
    * @param str the string to encode.
    * 
    * @return the encoded string.
    * 
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
    * 
    * @return the encoded byte array.
    * 
    * @throws IOException
    */
   public byte[] encodeHttpContentData(byte[] data) throws IOException
   {
      // get byte array input stream
      ByteArrayInputStream bias = new ByteArrayInputStream(data);
      
      // get encoded stream
      InputStream is = getHttpContentEncodedStream(
         getSupportedContentEncoding(), bias);
      
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
      
      // close output stream
      baos.close();
      
      return baos.toByteArray();
   }
   
   /**
    * Gets an output stream to write decoded data with.
    * 
    * @param contentEncoding the content-encoding for the stream.
    * @param os the output stream to write the decoded data to.
    * 
    * @return the output stream to write decoded data with.
    * 
    * @throws IOException
    */
   public OutputStream getHttpContentDecodedStream(
      String contentEncoding, OutputStream os)
   throws IOException  
   {
      OutputStream rval = os;
      
      // see if the content-encoding is supported
      if(isContentEncodingSupported(contentEncoding))
      {
         // get decoded stream
         rval = getHttpContentDecodedStream(os);
      }
      
      return rval;
   }
   
   /**
    * Decodes the passed string of http content.
    * 
    * @param str the string to decode.
    * 
    * @return the decoded string.
    * 
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
    * 
    * @return the decoded byte array.
    * 
    * @throws IOException
    */
   public byte[] decodeHttpContentData(byte[] data) throws IOException
   {
      // get byte array input stream
      ByteArrayInputStream bias = new ByteArrayInputStream(data);
      
      // get byte array output stream
      ByteArrayOutputStream baos = new ByteArrayOutputStream();
      
      // get decoded stream
      OutputStream os = getHttpContentDecodedStream(
         getSupportedContentEncoding(), baos);
      
      // read in encoded data and write out decoded data
      byte[] buffer = new byte[65536];
      int numBytes = -1;
      while((numBytes = bias.read(buffer)) != -1)
      {
         os.write(buffer, 0, numBytes);
         os.flush();
      }
      
      // close input stream
      bias.close();
      
      // close output stream
      os.close();
      
      return baos.toByteArray();      
   }
   
   /**
    * Gets the supported content-encoding.
    * 
    * @return the supported content-encoding.
    */
   public String getSupportedContentEncoding()
   {
      return mSupportedContentEncoding;
   }
}
