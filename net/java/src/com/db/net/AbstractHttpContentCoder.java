/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

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
    * The supported encoding.
    */
   protected String mSupportedEncoding;
   
   /**
    * Creates a new AbstractHttpContentCoder object.
    * 
    * @param supportedEncoding the supported encoding.
    */
   public AbstractHttpContentCoder(String supportedEncoding)
   {
      mSupportedEncoding = supportedEncoding;
   }
   
   /**
    * Checks to see if the passed encoding is supported.
    * 
    * @param encoding the encoding to check.
    * 
    * @return true if the passed encoding is supported, false if not.
    */
   protected boolean isEncodingSupported(String encoding)
   {
      boolean rval = false;
      
      rval = getSupportedEncoding().equals(encoding);
      
      return rval;
   }
   
   /**
    * Gets an input stream to read encoded data from.
    * 
    * @param encoding the content-encoding or transfer-encoding for the stream.
    * @param decodedStream the input stream with the data to encode.
    * 
    * @return the input stream to read encoded data with.
    * 
    * @throws IOException
    */
   public InputStream getHttpContentEncodedStream(
      String encoding, InputStream decodedStream)
   throws IOException
   {
      InputStream rval = decodedStream;
      
      // see if the encoding is supported
      if(isEncodingSupported(encoding))
      {
         // get encoded stream
         rval = getHttpContentEncodedStream(decodedStream);
      }
      
      return rval;
   }
   
   /**
    * Gets an input stream to read encoded data from.
    * 
    * @param decodedStream the input stream with the data to encode.
    * 
    * @return the input stream to read encoded data with.
    * 
    * @throws IOException
    */
   public abstract InputStream getHttpContentEncodedStream(
      InputStream decodedStream)
   throws IOException;
   
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
         getSupportedEncoding(), bias);
      
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
    * @param encoding the content-encoding or transfer-encoding for the stream.
    * 
    * @param encodedStream the input stream with the data to decode.
    * 
    * @return the input stream to read decoded data from.
    * 
    * @throws IOException
    */
   public InputStream getHttpContentDecodedStream(
      String encoding, InputStream encodedStream)
   throws IOException
   {
      InputStream rval = encodedStream;
      
      // see if the encoding is supported
      if(isEncodingSupported(encoding))
      {
         // get decoded stream
         rval = getHttpContentDecodedStream(encodedStream);
      }
      
      return rval;
   }
   
   /**
    * Gets an input stream to read decoded data from. This input stream must
    * not read more data than it needs because the current design filters
    * input from an http web connection directly into this stream.
    * 
    * @param encodedStream the input stream with the data to decode.
    * 
    * @return the input stream to read decoded data from.
    * 
    * @throws IOException
    */
   public abstract InputStream getHttpContentDecodedStream(
      InputStream encodedStream)
   throws IOException;   
   
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
      
      // get decoded stream
      InputStream is = getHttpContentDecodedStream(
         getSupportedEncoding(), bias);

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
   
   /**
    * Gets the supported encoding.
    * 
    * @return the supported encoding.
    */
   public String getSupportedEncoding()
   {
      return mSupportedEncoding;
   }
}
