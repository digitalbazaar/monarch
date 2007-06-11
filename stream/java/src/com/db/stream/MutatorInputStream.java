/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.IOException;
import java.io.FilterInputStream;
import java.io.InputStream;

import com.db.util.ByteBuffer;

/**
 * A MutatorInputStream uses a DataMutator with a specific DataMutationAlgorithm
 * to mutate data as it is read.
 * 
 * @author Dave Longley
 */
public class MutatorInputStream extends FilterInputStream
{
   /**
    * An internal buffer for storing data read from the underlying stream.
    */
   protected ByteBuffer mReadBuffer;
   
   /**
    * An internal buffer for storing mutated data.
    */
   protected ByteBuffer mMutatedData;
   
   /**
    * A simple read buffer for doing quick read()s.
    */
   protected byte[] mOneByteBuffer;
   
   /**
    * The DataMutator for this stream.
    */
   protected DataMutator mMutator;
   
   /**
    * Creates a new MutatorInputStream that mutates data with the passed
    * DataMutationAlgorithm.
    * 
    * @param is the underlying InputStream to read from.
    * @param algorithm the DataMutationAlgorithm to use.
    */
   public MutatorInputStream(InputStream is, DataMutationAlgorithm algorithm)
   {
      // store underlying stream
      super(is);
      
      // create read buffer
      mReadBuffer = new ByteBuffer(2048);
      
      // create mutated buffer
      mMutatedData = new ByteBuffer(4096);
      
      // create one byte buffer
      mOneByteBuffer = new byte[1];
      
      // create data mutator
      mMutator = new DataMutator(mReadBuffer, mMutatedData);
      
      // set the mutation algorithm
      mMutator.setAlgorithm(algorithm);
   }
   
   /**
    * Reads a single byte and returns it. If the end of the stream has been
    * reached, then -1 will be returned. This method will block until a
    * byte can be read or until the end of the stream is reached.
    *  
    * @return the byte read or -1 if end of the stream was reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   @Override
   public int read() throws IOException   
   {
      int rval = -1;
      
      // read into the one byte buffer
      if(read(mOneByteBuffer, 0, 1) != -1)
      {
         rval = mOneByteBuffer[0] & 0xff;
      }
      
      return rval;
   }
   
   /**
    * Reads up to length bytes of data from this input stream. The number of
    * bytes read will be returned unless the end of the stream is reached,
    * in which case -1 will be returned. This method will block until at least
    * one byte can be read or until the end of the stream is reached.
    * 
    * @param b the data buffer to read data into.
    * @param offset the offset at which to store the read data.
    * @param length the maximum number of bytes to read.
    * 
    * @return the number of bytes read or -1 if the end of the stream was
    *         reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   @Override
   public int read(byte[] b, int offset, int length) throws IOException
   {
      int rval = -1;
      
      // mutate data
      if(mMutator.mutate(in))
      {
         // get data from the mutator
         rval = mMutator.get(b, offset, length);
      }
      
      return rval;
   }
   
   /**
    * Skips up to the passed number of bytes. The actual number of bytes
    * skipped will be returned. This method will block until at least one
    * byte can be skipped or until the end of the stream is reached.
    *
    * @param n the number of bytes to be skipped.
    * 
    * @return the actual number of bytes skipped or -1 if the end of the
    *         stream is reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   @Override
   public long skip(long n) throws IOException
   {
      long rval = -1;
      
      // read into dummy buffer
      byte[] b = new byte[2048];
      int numBytes = Math.max(b.length, (int)n);
      while(n > 0 && (numBytes = read(b, 0, numBytes)) != -1)
      {
         n -= numBytes;
         rval = (rval == -1) ? numBytes : rval + numBytes;
         numBytes = Math.max(b.length, (int)n);
      }
      
      return rval;
   }
}
