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
public class DataMutator implements DataMutationAlgorithm
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
    * True if the data mutation algorithm has been called with the finish
    * flag set, false if not.
    */
   protected boolean mAlgorithmFinished;
   
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
      
      // no algorithm yet, use default
      mAlgorithm = this;
      mAlgorithmFinished = false;
   }
   
   /**
    * Sets the data mutation algorithm for this mutator.
    * 
    * @param algorithm the DataMutationAlgorithm to use.
    */
   public void setAlgorithm(DataMutationAlgorithm algorithm)
   {
      mAlgorithm = algorithm;
      mAlgorithmFinished = false;
   }
   
   /**
    * Runs a mutation algorithm on data read from the passed input stream
    * until some mutated bytes are available.
    * 
    * This method will block until this DataMutator has mutated data that can
    * be retrieved or until the end of the input stream has been reached and
    * there is no more mutated data available.
    * 
    * @param is the input stream to read from.
    * 
    * @return true if mutated data is available, false if the end of the
    *         stream has been reached and no more mutated data is available.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public boolean mutate(InputStream is) throws IOException
   {
      // mutate while no data is available and not finished
      boolean read = mSource.isEmpty();
      while(!hasData() && !mAlgorithmFinished)
      {
         // read as necessary
         if(read)
         {
            mAlgorithmFinished = (mSource.put(is) == -1);
         }
         
         // try to mutate data
         read = !mAlgorithm.mutateData(
            mSource, mDestination, mAlgorithmFinished);
      }
      
      return hasData();
   }
   
   /**
    * Runs a mutation algorithm on data read from the passed input stream
    * and skips the mutated bytes that are generated.
    * 
    * This method will block until this DataMutator has skipped the some
    * mutated data (it will try to skip the requested amount) or until the
    * end of the input stream has been reached and there is no more mutated
    * data to skip.
    * 
    * @param is the input stream to read from.
    * @param count the number of mutated bytes to skip. 
    * 
    * @return the number of skipped mutated bytes.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public long skipMutatedBytes(InputStream is, long count)
   throws IOException
   {
      long rval = 0;
      
      // mutate and skip data
      long remaining = count;
      while(remaining > 0 && mutate(is))
      {
         // clear bytes
         remaining -= mDestination.clear((int)remaining);
      }
      
      if(remaining < count)
      {
         // some bytes were skipped
         rval = count - remaining;
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
    * @param finish true to finish the mutation algorithm, false not to.
    * 
    * @return true if there was enough data in the source buffer to run the
    *         mutation algorithm (which may or may not produce mutated bytes).
    */
   public boolean mutateData(ByteBuffer src, ByteBuffer dest, boolean finish)   
   {
      boolean rval = false;
      
      if(!mSource.isEmpty())
      {
         // get all data from source and put it in destination
         mSource.get(mDestination, mSource.getUsedSpace(), true);
         rval = true;
      }
      
      return rval;
   }
}
