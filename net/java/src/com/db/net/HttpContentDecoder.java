/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.InputStream;

/**
 * A class that implements the HttpContentDecoder interface can be used
 * to decode http message bodies according to their content-encoding or
 * transfer-encoding.
 * 
 * @author Dave Longley
 */
public interface HttpContentDecoder
{
   /**
    * Gets an input stream to read decoded data from. This input stream must
    * not read more data than it needs because the current design filters
    * input from an http web connection directly into this stream.
    * 
    * @param encoding the content-encoding or transfer-encoding for the stream.
    * @param encodedStream the input stream with the data to decode.
    * 
    * @return the input stream to read decoded data from.
    * 
    * @throws IOException
    */
   public InputStream getHttpContentDecodedStream(
      String encoding, InputStream encodedStream)
   throws IOException;   
   
   /**
    * Decodes the passed string of http content.
    * 
    * @param str the string to decode.
    * 
    * @return the decoded string.
    * 
    * @throws IOException
    */
   public String decodeHttpContentString(String str) throws IOException;
   
   /**
    * Decodes the passed byte array of http content.
    * 
    * @param data the byte array to decode.
    * 
    * @return the decoded byte array.
    * 
    * @throws IOException
    */
   public byte[] decodeHttpContentData(byte[] data) throws IOException;
}
