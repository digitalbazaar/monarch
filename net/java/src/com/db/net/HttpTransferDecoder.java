/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * A class that implements the HttpTransferDecoder interface can be used
 * to decode http message bodies according to their transfer-encoding.
 * 
 * @author Dave Longley
 */
public interface HttpTransferDecoder
{
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
   throws IOException;
}
