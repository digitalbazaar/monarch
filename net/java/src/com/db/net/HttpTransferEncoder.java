/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;

/**
 * A class that implements the HttpTransferEncoder interface can be used
 * to encode http message bodies according to their transfer-encoding.
 * 
 * @author Dave Longley
 */
public interface HttpTransferEncoder
{
   /**
    * Reads data from the passed input stream until the end of the stream,
    * encodes the data according to the transfer-encoding in the passed
    * http header, and then writes the data out to the passed http
    * web connection.
    * 
    * @param header the http header for the http body.
    * @param bodyStream the input stream with the body to encode.
    * @param hwc the http web connection to write the http body to.
    * 
    * @return the total number of bytes written.
    * 
    * @throws IOException
    * @throws InterruptedException
    */
   public long writeHttpBody(
      HttpHeader header, InputStream bodyStream, HttpWebConnection hwc)
   throws IOException, InterruptedException;
}
