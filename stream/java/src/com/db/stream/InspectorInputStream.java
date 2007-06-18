/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.IOException;
import java.io.FilterInputStream;
import java.io.InputStream;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import com.db.util.ByteBuffer;

/**
 * An InspectorInputStream uses DataInspectors to collect information about
 * the data that is read.
 * 
 * @author Dave Longley
 */
public class InspectorInputStream extends FilterInputStream
{
   /**
    * The data inspectors (stored along with their meta-data).
    */
   protected HashMap<String, DataInspectorMetaData> mInspectors;
   
   /**
    * Stores the data inspectors that are waiting to inspect the currently
    * buffered data.
    */
   protected List<DataInspectorMetaData> mWaiting;
   
   /**
    * An internal buffer for storing data read from the underlying stream.
    */
   protected ByteBuffer mReadBuffer;
   
   /**
    * Stores the number of bytes that have been inspected and can be
    * released from the read buffer.
    */
   protected int mAvailableBytes;
   
   /**
    * A simple read buffer for doing quick read()s.
    */
   protected byte[] mOneByteBuffer;
   
   /**
    * Creates a new InspectorInputStream.
    * 
    * @param is the underlying InputStream to read from.
    */
   public InspectorInputStream(InputStream is)
   {
      // store underlying stream
      super(is);
      
      // create inspectors collection
      mInspectors = new HashMap<String, DataInspectorMetaData>();
      
      // create list of inspectors that are waiting to inspect
      mWaiting = new LinkedList<DataInspectorMetaData>();
      
      // create read buffer
      mReadBuffer = new ByteBuffer(2048);
      
      // no bytes inspected and made available yet
      mAvailableBytes = 0;
      
      // create one byte buffer
      mOneByteBuffer = new byte[1];
   }
   
   /**
    * Adds an inspector to this stream.
    * 
    * @param name the name for the inspector.
    * @param di the DataInspector to add.
    */
   public void addInspector(String name, DataInspector di)
   {
      // create meta-data
      DataInspectorMetaData metaData = new DataInspectorMetaData(di);
      
      // store meta-data
      mInspectors.put(name, metaData);
   }
   
   /**
    * Removes an inspector from this stream.
    * 
    * @param name the name of the DataInspector to remove.
    */
   public void removeInspector(String name)
   {
      mInspectors.remove(name);
   }
   
   /**
    * Gets a DataInspector from this stream by its name.
    * 
    * @param name the name of the DataInspector to get.
    * 
    * @return the DataInspector with the given name or null if none exists.
    */
   public DataInspector getInspector(String name)
   {
      DataInspector rval = null;
      
      DataInspectorMetaData metaData = mInspectors.get(name);
      if(metaData != null)
      {
         rval = metaData.getInspector();
      }
      
      return rval;
   }
   
   /**
    * Gets all of the DataInspectors from this stream.
    * 
    * @return all of the DataInspectors from this stream.
    */
   public Collection<DataInspector> getInspectors()
   {
      LinkedList<DataInspector> list = new LinkedList<DataInspector>();
      
      for(DataInspectorMetaData metaData: mInspectors.values())
      {
         list.add(metaData.getInspector());
      }
      
      return list;
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
      
      // if the read buffer is empty, populate it
      if(mReadBuffer.isEmpty())
      {
         mReadBuffer.put(in);
      }
      
      // if no bytes are available, run inspectors to release inspected bytes
      if(mAvailableBytes == 0)
      {
         // add all inspector meta-data to the waiting list
         mWaiting.addAll(mInspectors.values());
         
         // reset the number of available bytes to the maximum
         mAvailableBytes = mReadBuffer.getUsedSpace();
         
         // keep inspecting while inspectors are waiting
         while(!mWaiting.isEmpty() && !mReadBuffer.isEmpty())
         {
            // run waiting inspectors
            for(Iterator<DataInspectorMetaData> i = mWaiting.iterator();
                i.hasNext();)
            {
               // get next meta data
               DataInspectorMetaData metaData = i.next();
               
               // determine the number of uninspected bytes
               int uninspectedBytes =
                  mReadBuffer.getUsedSpace() - metaData.getInspectedBytes();
               if(uninspectedBytes > 0)
               {
                  // inspect data using inspector
                  int inspected = metaData.getInspector().inspectData(
                     mReadBuffer.getBytes(),
                     mReadBuffer.getOffset() + metaData.getInspectedBytes(),
                     uninspectedBytes);
                  
                  // see if any data was inspected
                  if(inspected > 0)
                  {
                     // update number of inspected bytes
                     metaData.setInspectedBytes(
                        metaData.getInspectedBytes() + inspected);
                     
                     // update the amount of available bytes
                     mAvailableBytes = Math.min(
                        mAvailableBytes, metaData.getInspectedBytes());
                     
                     // inspector is no longer waiting
                     i.remove();
                  }
               }
               else
               {
                  // remove inspector from list, all current data inspected
                  i.remove();
               }
            }
            
            // remove all waiting inspectors if the read buffer is full
            if(mReadBuffer.isFull())
            {
               mWaiting.clear();
            }
            else if(!mWaiting.isEmpty())
            {
               // read more data into the read buffer
               mReadBuffer.put(in);
            }
         }
      }
      
      // if bytes are available, release them
      if(mAvailableBytes > 0)
      {
         // pull bytes from the read buffer
         rval = mReadBuffer.get(
            b, offset, Math.min(length, mAvailableBytes));
         mAvailableBytes -= rval;
         
         // update the number of inspected bytes in each inspector
         for(DataInspectorMetaData metaData: mInspectors.values())
         {
            metaData.setInspectedBytes(metaData.getInspectedBytes() - rval);
         }
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
   
   /**
    * A helper class that maintains information about a particular
    * DataInspector.
    * 
    * @author Dave Longley
    */
   public class DataInspectorMetaData
   {
      /**
       * The DataInspector.
       */
      protected DataInspector mInspector;
      
      /**
       * The current number of bytes that have been inspected by the associated
       * inspector in the shared read buffer.
       */
      protected int mInspectedBytes;
      
      /**
       * Creates a new DataInspectorMetaData.
       * 
       * @param di the DataInspector.
       */
      public DataInspectorMetaData(DataInspector di)
      {
         // store inspector
         mInspector = di;
         
         // set inspected bytes
         mInspectedBytes = 0;
      }
      
      /**
       * Gets the DataInspector.
       * 
       * @return the DataInspector.
       */
      public DataInspector getInspector()
      {
         return mInspector;
      }
      
      /**
       * Sets the current number of bytes inspected in the shared read buffer.
       * 
       * @param count the current number of inspected bytes.
       */
      public void setInspectedBytes(int count)
      {
         mInspectedBytes = count;
      }
      
      /**
       * Gets the current number of bytes inspected in the shared read buffer.
       * 
       * @return the current number of inspected bytes.
       */
      public int getInspectedBytes()
      {
         return mInspectedBytes;
      }
   }
}
