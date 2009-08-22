/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.http;

import java.io.IOException;
import java.io.InputStream;

/**
 * A class that implements the HttpContentEncoder interface can be used
 * to encode http message bodies according to their content-encoding.
 * 
 * @author Dave Longley
 */
public interface HttpContentEncoder
{
   /**
    * Gets an input stream to read encoded data from.
    * 
    * @param contentEncoding the content-encoding for the stream.
    * @param unencodedStream the input stream with the data to encode.
    * 
    * @return the input stream to read encoded data from.
    * 
    * @throws IOException
    */
   public InputStream getHttpContentEncodedStream(
      String contentEncoding, InputStream unencodedStream)
   throws IOException;
   
   /**
    * Encodes the passed string of http content.
    * 
    * @param str the string to encode.
    * 
    * @return the encoded string.
    * 
    * @throws IOException
    */
   public String encodeHttpContentString(String str) throws IOException;
   
   /**
    * Encodes the passed byte array of http content.
    * 
    * @param data the byte array to encode.
    * 
    * @return the encoded byte array.
    * 
    * @throws IOException
    */
   public byte[] encodeHttpContentData(byte[] data) throws IOException;
}
