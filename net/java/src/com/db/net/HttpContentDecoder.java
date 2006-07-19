/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.IOException;
import java.io.OutputStream;

/**
 * A class that implements the HttpContentDecoder interface can be used
 * to decode http message bodies according to their content-encoding.
 * 
 * @author Dave Longley
 */
public interface HttpContentDecoder
{
   /**
    * Gets an output stream to write decoded data with.
    * 
    * @param contentEncoding the content-encoding for the stream.
    * @param os the output stream to write the decoded data to.
    * 
    * @return the output stream to write decoded data with.
    * 
    * @throws IOException
    */
   public OutputStream getHttpContentDecodedStream(
      String contentEncoding, OutputStream os)
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
