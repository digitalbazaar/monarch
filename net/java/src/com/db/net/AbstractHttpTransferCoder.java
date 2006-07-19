/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * An AbstractHttpTransferCoder is a class that is used to encode and decode
 * http message body transfers for http web requests and and http web
 * responses.
 * 
 * It implements some of the basic methods for HttpTransferEncoder and
 * HttpTransferEncoder to allow easier extending.
 * 
 * @author Dave Longley
 */
public abstract class AbstractHttpTransferCoder
implements HttpTransferEncoder, HttpTransferDecoder
{
   /**
    * The supported transfer-encoding.
    */
   protected String mSupportedTransferEncoding;
   
   /**
    * Creates a new AbstractHttpTransferCoder.
    * 
    * @param supportedTransferEncoding the supported transfer-encoding.
    */
   public AbstractHttpTransferCoder(String supportedTransferEncoding)
   {
      mSupportedTransferEncoding = supportedTransferEncoding;
   }
   
   /**
    * Checks to see if the passed transfer-encoding is supported.
    * 
    * @param transferEncoding the transfer-encoding to check.
    * 
    * @return true if the passed transfer-encoding is supported, false if not.
    */
   protected boolean isTransferEncodingSupported(String transferEncoding)
   {
      boolean rval = false;
      
      if(transferEncoding != null)
      {
         rval = transferEncoding.startsWith(getSupportedTransferEncoding());
      }
      
      return rval;
   }
   
   /**
    * Reads data from the passed input stream until the end of the stream,
    * encodes the data according to the supported transfer-encoding, and
    * then writes the data out to the passed output stream.
    * 
    * @param bodyStream the input stream with the body to encode.
    * @param os the output stream to write the encoded body to.
    * 
    * @return the total number of bytes written.
    * 
    * @throws IOException
    */
   protected abstract long writeHttpBody(
      InputStream bodyStream, OutputStream os)
   throws IOException;
   
   /**
    * Reads and decodes an http message body from the passed input stream,
    * according to the supported transfer-encoding, and writes it to the
    * passed output stream, unless the output stream is null.
    *  
    * @param bodyStream the input stream to read the http body from. 
    * @param os the output stream to write the body to.
    * 
    * @return the total number of bytes read.
    * 
    * @throws IOException
    */
   protected abstract long readHttpBody(
      InputStream bodyStream, OutputStream os)
   throws IOException;
   
   /**
    * Reads data from the passed input stream until the end of the stream,
    * encodes the data according to the passed transfer-encoding, and
    * then writes the data out to the passed output stream.
    * 
    * @param transferEncoding the transfer-encoding to encode according to.
    * @param bodyStream the input stream with the body to encode.
    * @param os the output stream to write the encoded body to.
    * 
    * @return the total number of bytes written.
    * 
    * @throws IOException
    * @throws IllegalArgumentException
    */
   public long writeHttpBody(
      String transferEncoding, InputStream bodyStream, OutputStream os)
   throws IOException, IllegalArgumentException
   {
      long rval = 0;
      
      if(isTransferEncodingSupported(transferEncoding))
      {
         rval = writeHttpBody(bodyStream, os);
      }
      else
      {
         throw new IllegalArgumentException(
            "Passed transfer-encoding is not supported!");
      }
      
      return rval;
   }

   /**
    * Reads and decodes an http message body from the passed input stream,
    * according to the passed transfer-encoding, and writes it to the
    * passed output stream, unless the output stream is null.
    *  
    * @param transferEncoding the transfer-encoding to decode according to.
    * @param bodyStream the input stream to read the http body from. 
    * @param os the output stream to write the body to.
    * 
    * @return the total number of bytes read.
    * 
    * @throws IOException
    */
   public long readHttpBody(
      String transferEncoding, InputStream bodyStream, OutputStream os)
   throws IOException
   {
      long rval = 0;
      
      if(isTransferEncodingSupported(transferEncoding))
      {
         rval = readHttpBody(bodyStream, os);
      }
      else
      {
         throw new IllegalArgumentException(
            "Passed transfer-encoding is not supported!");
      }
      
      return rval;
   }
   
   /**
    * Gets the supported transfer-encoding.
    * 
    * @return the supported transfer-encoding.
    */
   public String getSupportedTransferEncoding()
   {
      return mSupportedTransferEncoding;
   }   
}
