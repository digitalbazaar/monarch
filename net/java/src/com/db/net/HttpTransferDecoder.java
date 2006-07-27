/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
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
    * Reads and decodes an http message body from the passed http web
    * connection, according to the transfer-encoding in the passed
    * http header, and writes it to the passed output stream, unless
    * the output stream is null.
    * 
    * @param header the http header for the http body.
    * @param hwc the http web connection to read the http body from. 
    * @param os the output stream to write the body to.
    * 
    * @return the total number of content bytes read.
    * 
    * @throws IOException
    * @throws InterruptedException
    */
   public long readHttpBody(
      HttpHeader header, HttpWebConnection hwc, OutputStream os)
   throws IOException, InterruptedException;
}
