/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;

/**
 * A ChunkedHttpContentCoder is a class that is used to encode and decode
 * http message body content for http web requests and and http web responses
 * that have a transfer-encoding header value set to "chunked".  
 * 
 * @author Dave Longley
 */
public class ChunkedHttpContentCoder extends AbstractHttpContentCoder
{
   /**
    * Creates a new ChunkedHttpContentCoder object.
    */
   public ChunkedHttpContentCoder()
   {
      super("chunked");
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
   public InputStream getHttpContentEncodedStream(InputStream decodedStream)
   throws IOException
   {
      InputStream rval = decodedStream;
      
      // FIXME:
      // create chunked transfer-encoding encoder stream
      
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
   public InputStream getHttpContentDecodedStream(InputStream encodedStream)
   throws IOException
   {
      InputStream rval = encodedStream;
      
      // FIXME:
      // create chunked transfer-encoding encoder stream
      
      return rval;
   }
}
