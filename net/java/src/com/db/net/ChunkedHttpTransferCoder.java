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
    * then writes the data out to the passed output stream.
    * 
    * @param bodyStream the input stream with the body to encode.
    * @param os the output stream to write the encoded body to.
    * 
    * @return the total number of bytes written.
    * 
    * @throws IOException
    */
   protected long writeHttpBody(InputStream bodyStream, OutputStream os)
   throws IOException
   {
      long rval = 0;
      
      // FIXME:
      
      return rval;
   }
   
   /**
    * Reads and decodes an http message body from the passed input stream,
    * according to the supported transfer-encoding, and writes it to the
    * passed output stream.
    *  
    * @param bodyStream the input stream to read the http body from. 
    * @param os the output stream to write the body to.
    * 
    * @return the total number of bytes read.
    * 
    * @throws IOException
    */
   protected long readHttpBody(InputStream bodyStream, OutputStream os)
   throws IOException
   {
      long rval = 0;
      
      // FIXME:
      
      return rval;
   }
}
