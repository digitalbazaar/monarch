/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

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
    */
   public long writeHttpBody(
      String transferEncoding, InputStream bodyStream, OutputStream os)
   throws IOException;
}
