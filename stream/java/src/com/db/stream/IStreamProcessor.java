/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

/**
 * This interface allows an implementing class to process and/or
 * make changes to streaming data as it is streamed.
 * 
 * @author Dave Longley
 */
public interface IStreamProcessor
{
   /**
    * Processes the passed data and returns the resulting processed data.
    * 
    * The second parameter indicates whether the passed data is the
    * last data in the stream that is using this interface.
    * 
    * @param data the data to process.
    * @param last whether or not the passed data is the last data
    *        in the associated stream. 
    * @return the processed data, null if no data to process for the last read.
    */
   public byte[] processStreamData(byte[] data, boolean last);
}
