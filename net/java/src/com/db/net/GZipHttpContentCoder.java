/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import com.db.stream.PipeInputStream;

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
public class GZipHttpContentCoder extends AbstractHttpContentCoder
{
   /**
    * Creates a new GZipHttpContentCoder object.
    */
   public GZipHttpContentCoder()
   {
      super("gzip");
   }
   
   /**
    * Gets an input stream to read encoded data from.
    * 
    * @param contentEncoding the content encoding for the stream.
    * @param decodedStream the input stream with the data to encode.
    * 
    * @return the input stream to read encoded data with.
    * 
    * @throws IOException
    */
   public InputStream getHttpContentEncodedStream(
      String contentEncoding, InputStream decodedStream)
   throws IOException
   {
      InputStream rval = decodedStream;
      
      if(isEncodingSupported(contentEncoding))
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
    * Gets an input stream to read decoded data from. This input stream must
    * not read more data than it needs because the current design filters
    * input from an http web connection directly into this stream.
    * 
    * @param contentEncoding the content encoding for the stream.
    * 
    * @param encodedStream the input stream with the data to decode.
    * 
    * @return the input stream to read decoded data from.
    * 
    * @throws IOException
    */
   public InputStream getHttpContentDecodedStream(
      String contentEncoding, InputStream encodedStream)
   throws IOException
   {
      InputStream rval = encodedStream;
      
      if(isEncodingSupported(contentEncoding))
      {
         // wrap the input stream with a gzip input stream
         GZIPInputStream gzis = new GZIPInputStream(encodedStream);
         
         // set gzip stream as input stream
         rval = gzis;
      }
      
      return rval;
   }
}
