/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import com.db.stream.GZipInputStream;
import com.db.stream.UnGZipOutputStream;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

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
    * @param unencodedStream the input stream with the data to encode.
    * 
    * @return the input stream to read encoded data with.
    * 
    * @throws IOException
    */
   protected InputStream getHttpContentEncodedStream(
      InputStream unencodedStream)
   throws IOException
   {
      InputStream rval = unencodedStream;
      
      // create a pipe input stream that reads from the passed stream
      com.db.stream.PipeInputStream pipe = new com.db.stream.PipeInputStream(unencodedStream);
      
      // wrap the output stream that writes to the pipe
      java.util.zip.GZIPOutputStream gzos = new java.util.zip.GZIPOutputStream(pipe.getOutputStream());
      
      // attach the output stream to the pipe
      pipe.setOutputStream(gzos);
      
      // set pipe as input stream
      rval = pipe;      
      
      return rval;
      
      // return a gzip input stream
      //return new GZipInputStream(unencodedStream);
   }
   
   /**
    * Gets an output stream to write decoded data with.
    * 
    * @param os the output stream to write the decoded data to.
    * 
    * @return the output stream to write decoded data with.
    * 
    * @throws IOException
    */
   protected OutputStream getHttpContentDecodedStream(OutputStream os)
   throws IOException   
   {
      // return a gzip output stream
      return new UnGZipOutputStream(os);
   }
}
