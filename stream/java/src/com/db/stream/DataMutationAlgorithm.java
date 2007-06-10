/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import com.db.util.ByteBuffer;

/**
 * A DataMutationAlgorithm is used to mutate data in a source ByteBuffer and
 * write the mutated bytes to a destinatino ByteBuffer.
 *  
 * @author Dave Longley
 */
public interface DataMutationAlgorithm
{
   /**
    * Gets data out of the source ByteBuffer, mutates it in some implementation
    * specific fashion, and then puts it in the destination ByteBuffer. The
    * actual number of mutated bytes is returned, which may be zero if there
    * are not enough bytes in the source buffer to produce mutated bytes.
    * 
    * Note: The destination buffer will be resized to accommodate any mutated
    * bytes.
    * 
    * @param src the source ByteBuffer with bytes to mutate.
    * @param dest the destination ByteBuffer to write the mutated bytes to.
    */
   public void mutateData(ByteBuffer src, ByteBuffer dest);
}
