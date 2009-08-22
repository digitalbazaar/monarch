/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

/**
 * A DataInspector is used to inspect data for some implementation specific
 * attributes.
 *  
 * @author Dave Longley
 */
public interface DataInspector
{
   /**
    * Inspects the data in the passed buffer for some implementation
    * specific attributes. This method returns the number of bytes that
    * were successfully inspected such that the passed buffer can safely
    * clear that number of bytes. 
    * 
    * An inspector should treat subsequent calls to this method as if the
    * data in the passed buffer is consecutive in nature (i.e. read
    * from a stream).
    * 
    * @param b the buffer with data to inspect.
    * @param offset the offset at which to begin inspecting bytes.
    * @param length the maximum number of bytes to inspect.
    * 
    * @return the number of bytes that were inspected in the passed buffer.
    */
   public int inspectData(byte[] b, int offset, int length);
}
