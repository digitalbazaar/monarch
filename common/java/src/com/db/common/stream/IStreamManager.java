/*
 * Copyright (c) 2003 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.stream;

/**
 * This interface manages how much data is should be
 * obtained by a managed stream. It allows for data to be
 * obtained until there is enough of it to be processed by
 * an appropriate stream processor.
 * 
 * Exactly how much data is required before processing
 * is entirely dependent upon how the implementation of this
 * interface manages the data logically.
 * 
 * @author Dave Longley
 */
public interface IStreamManager
{
   /**
    * This method returns the number of additional bytes that are
    * required to process the passed data.
    * 
    * If the return value is negative, it is an indication that the
    * managed stream that uses this interface has obtained too much
    * data. The managed stream should hold onto that extra data until
    * this method returns a positive number again.
    * 
    * If the return value is zero, it is an indication that enough
    * data has been obtained and the data is ready to be processed.
    * 
    * No data will be passed to a stream processor until this
    * method returns 0, a negative number, or the stream has no
    * more data.
    * 
    * @param data the data buffer with the that has been read so far.
    * @param offset the offset for valid data in the buffer.
    * @param length the amount of valid bytes in the data buffer.
    * @return the positive number of additional bytes required before
    *         the data can be processed, or the negative number of
    *         extra bytes to be saved until this method requests them, or
    *         zero indicating that the data is ready to be processed.
    */
   public int manageStreamData(byte[] data, int offset, int length);
}
