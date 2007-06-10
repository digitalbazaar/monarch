/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.IOException;
import java.io.InputStream;

import com.db.util.ByteBuffer;

/**
 * A DataMutator mutates data that is stored in a ByteBuffer and writes it to
 * another ByteBuffer from which it can be retrieved.
 * 
 * @author Dave Longley
 */
public class DataMutator
{
   /**
    * The source ByteBuffer with unmutated data.
    */
   protected ByteBuffer mSource;
   
   /**
    * The destination ByteBuffer with mutated data.
    */
   protected ByteBuffer mDestination;
   
   /**
    * The algorithm used to mutate data.
    */
   protected DataMutationAlgorithm mAlgorithm;
   
   /**
    * Creates a new DataMutator.
    * 
    * @param src the source ByteBuffer to read from.
    * @param dest the destination ByteBuffer to write to.
    */
   public DataMutator(ByteBuffer src, ByteBuffer dest)
   {
      // store buffers
      mSource = src;
      mDestination = dest;
      
      // no algorithm yet
      mAlgorithm = null;
   }
   
   /**
    * Sets the data mutation algorithm for this mutator.
    * 
    * @param algorithm the DataMutationAlgorithm to use.
    */
   public void setAlgorithm(DataMutationAlgorithm algorithm)
   {
      mAlgorithm = algorithm;
   }
   
   /**
    * Reads data from the passed input stream and mutates it.
    * 
    * This method will block until at one byte can be read from the input
    * stream, unless this source buffer for this mutator is already full, or
    * until the end of the stream has been reached.
    * 
    * @param is the input stream to read from.
    * 
    * @return the number of bytes read from the input stream and put into
    *         this mutator, or -1 if the end of the input stream was reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public int put(InputStream is) throws IOException
   {
      int rval = mSource.put(is);
      
      // only mutate data if the destination is empty
      if(mDestination.isEmpty())
      {
         if(mAlgorithm != null)
         {
            // mutate the data
            mAlgorithm.mutateData(mSource, mDestination);
         }
         else
         {
            // get all data from source and put it in destination
            mSource.get(mDestination, mSource.getUsedSpace(), true);
         }
      }
      
      return rval;
   }
   
   /**
    * Gets data out of this mutator and puts it into the passed buffer. The
    * amount of data may be less than the requested amount if this mutator
    * does not have enough data.
    * 
    * @param b the buffer to put the retrieved data into.
    * @param offset the offset at which to put the retrieved data.
    * @param length the maximum number of bytes to get.
    * 
    * @return the actual number of bytes retrieved, which may be 0 if this
    *         mutator is empty.
    */
   public int get(byte[] b, int offset, int length)
   {
      return mDestination.get(b, offset, length);
   }
   
   /**
    * Returns true if this mutator has data that can be obtained, false if
    * not.
    * 
    * @return true if this mutator has data that can be obtained, false if
    *         not.
    */
   public boolean hasData()
   {
      return !mDestination.isEmpty();
   }
}
