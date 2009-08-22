/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.http;

import com.db.stream.GzipInputStream;
import com.db.stream.UnGzipOutputStream;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * A GzipHttpContentCoder is a class that is used to encode and decode
 * http message body content for http web requests and and http web responses
 * that have a content-encoding header value set to "gzip".  
 * 
 * @author Dave Longley
 */
public class GzipHttpContentCoder extends AbstractHttpContentCoder
{
   /**
    * Creates a new GzipHttpContentCoder object.
    */
   public GzipHttpContentCoder()
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
   @Override
   protected InputStream getHttpContentEncodedStream(
      InputStream unencodedStream)
   throws IOException
   {
      // return a gzip input stream
      return new GzipInputStream(unencodedStream);
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
   @Override
   protected OutputStream getHttpContentDecodedStream(OutputStream os)
   throws IOException   
   {
      // return a gzip output stream
      return new UnGzipOutputStream(os);
   }
}
