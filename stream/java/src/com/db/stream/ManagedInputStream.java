/*
 * Copyright (c) 2003-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.IOException;
import java.io.FilterInputStream;
import java.io.InputStream;

import com.db.logging.LoggerManager;

/**
 * FIXME: This class will soon be marked deprecated. It is way too memory
 * intensive and is being replaced with a simpler architecture of a
 * MutatorInputStream (for mutating data as it is read using implementation
 * specific algorithms) and a InspectorInputStream/FormatDetectionInputStream
 * for detecting data formats that has a very small memory footprint where
 * a series of DataInspectors/DataFormatDetectors share the same memory source. 
 * 
 * A managed input stream is a composition of a FilterInputStream and
 * two interfaces: IStreamManager and IStreamProcessor. It is an
 * input stream that allows the data that is read when any read()
 * method is called to be managed by classes that implement the
 * interfaces. 
 * 
 * The first interface is used to manage how much data must be read
 * from the underlying input stream until the second interface
 * can process that data in an appropriate fashion.
 * 
 * Once the IStreamManager decides that the input stream has
 * read enough data, the data is handed over to the IStreamProcessor
 * which processes the data and finally returns it so that it can
 * be given to the user of the input stream.
 * 
 * @author Dave Longley
 */
public class ManagedInputStream extends FilterInputStream
{
   /**
    * The interface used to manage the streaming data. 
    */
   protected IStreamManager mManager;
   
   /**
    * The interface used to process the streaming data. 
    */
   protected IStreamProcessor mProcessor;
   
   /**
    * A buffer that stores data that has been read but not yet
    * processed.
    */
   protected byte[] mReadBuffer;
   
   /**
    * The maximum size the read buffer is allowed to reach before
    * aborting.
    */
   protected int mMaxReadSize;
   
   /**
    * The current starting offset in the read buffer.
    */
   protected int mReadOffset;
   
   /**
    * The current position in the read buffer. This is the first index that
    * newly read data should be written to.
    */
   protected int mReadPos;
   
   /**
    * A buffer that stores data that has been read and processed.
    */
   protected byte[] mProcessedData;
   
   /**
    * The current position in the processed data buffer.
    */
   protected int mProcessedPos;
   
   /**
    * Set to true when the end of this stream has been reached.
    */
   protected boolean mEndOfStream;
   
   /**
    * The default read size for a managed read.
    */
   protected int DEFAULT_READ_SIZE = 2048;
   
   /**
    * The underlying input stream that this managed input stream is
    * linked to must be passed as a parameter.
    * 
    * @param inputStream the input stream that this stream is linked to.
    * @param manager the interface that manages reading data.
    * @param processor the interface that manages processing data.
    */
   public ManagedInputStream(InputStream inputStream,
                             IStreamManager manager,
                             IStreamProcessor processor)
   {
      super(inputStream);
      
      mManager = manager;
      mProcessor = processor;
      
      mReadBuffer = new byte[0];
      mMaxReadSize = -1;
      mReadOffset = 0;
      mReadPos = 0;
      
      mProcessedData = null;
      mProcessedPos = 0;
      
      mEndOfStream = false;
   }
   
   /**
    * Expands the read buffer maintaining its contents.
    * 
    * @param amount the amount to expand the buffer by.
    */
   protected void expandReadBuffer(int amount)
   {
      if(amount > 0)
      {
         byte[] newbuf = new byte[mReadBuffer.length + amount];
         System.arraycopy(mReadBuffer, mReadOffset,
                          newbuf, 0, mReadBuffer.length - mReadOffset);
         mReadBuffer = newbuf;
         mReadPos -= mReadOffset;
         mReadOffset = 0;
      }
   }
   
   /**
    * Reads into the read buffer the amount specified and
    * returns the actual amount read.
    * 
    * @param len the number of bytes to read.
    * 
    * @return the number of bytes read.
    * 
    * @throws IOException
    */
   protected int readIntoBuffer(int len) throws IOException
   {
      int numBytes = -1;
      
      if(mReadOffset != 0 && mReadPos == mReadOffset)
      {
         // reset the read offset and read position
         mReadPos = 0;
         mReadOffset = 0;
      }

      if(!mEndOfStream)
      {
         // expand the read buffer if necessary
         if(mReadBuffer.length < (mReadPos + len))
         {
            expandReadBuffer(len);
         }

         // determine the read size for this read
         int maxSize = mReadPos - mReadOffset + len;
         
         if(mMaxReadSize != -1)
         {
            maxSize = Math.min(maxSize, mMaxReadSize);
         }
         
         int readSize = maxSize - (mReadPos - mReadOffset); 

         // read if maximum read size not exceeded
         if(mMaxReadSize == -1 ||
            (readSize != 0 && readSize <= mMaxReadSize))
         {
            // read into read buffer starting at current read position
            numBytes = in.read(mReadBuffer, mReadPos, readSize);
            
            // break out if end of stream reached, else increment
            // read position
            if(numBytes == -1)
            {
               mEndOfStream = true;
            }
            else
            {
               mReadPos += numBytes;
            }
         }
         else
         {
            // abort, force end of stream
            mEndOfStream = true;
            numBytes = -1;
         }
      }
      
      return numBytes;
   }
   
   /**
    * Performs a managed read, preparing data for processing. Data will
    * be read until the manager indicates to stop or the end of the
    * stream is reached. This method returns null when there is no more
    * data in the stream to be processed.
    *
    * @param len the default read size to use. 
    * 
    * @return the data ready for processing or null if no more data.
    * 
    * @throws IOException
    */
   protected byte[] performManagedRead(int len) throws IOException
   {
      byte[] data = null;
      
      // perform a read if necessary
      if(len > (mReadPos - mReadOffset))
      {
         int size = len - (mReadPos - mReadOffset);
         readIntoBuffer(Math.max(DEFAULT_READ_SIZE, size));
      }
      
      // make sure there is data in the read buffer 
      if(mReadPos > 0)
      {
         // for storing the number of bytes requested by the manager
         int requiredBytes = 0;
            
         // keep reading until the manager indicates enough has been read
         while((requiredBytes = mManager.manageStreamData(
               mReadBuffer, mReadOffset, mReadPos - mReadOffset)) > 0)
         {
            // break out if end of stream reached
            if(readIntoBuffer(requiredBytes) == -1)
            {
               break;
            }
         }
         
         // set processed data if not end of stream or data is available
         if(!mEndOfStream || mReadPos != 0)
         {
            // get the length of the data to be processed
            int length = mReadPos - mReadOffset;
            
            // decrease the length as necessary
            if(requiredBytes < 0)
            {
               length += requiredBytes;
            }
            
            // allocate the data that is ready for processing
            data = new byte[length];
            System.arraycopy(mReadBuffer, mReadOffset, data, 0, length);
            
            // increment read offset
            mReadOffset += length;
            
            // reset read offset and read position if appropriate
            if(mReadOffset == mReadPos)
            {
               mReadPos = 0;
               mReadOffset = 0;
            }
         }
      }

      // return the data
      return data;
   }
   
   /**
    * Performs an unmanaged read, preparing data for processing. The
    * passed amount of data will be read or reading will continue to
    * the end of the stream, whichever comes first. This method returns
    * null when there is no more data in the stream to be processed.
    *
    * @param len the number of bytes to read. 
    * 
    * @return the data ready for processing or null if no more data.
    * 
    * @throws IOException
    */
   protected byte[] performUnmanagedRead(int len) throws IOException
   {
      byte[] data = null;
      
      if(!mEndOfStream)
      {
         // read the data without any manager
         data = new byte[len];
         int numBytes = in.read(data);

         // if there is data to process
         if(numBytes == -1)
         {
            mEndOfStream = true;
            data = null;
         }
         else if(numBytes < len)
         {
            // if full length couldn't be read, resize buffer
            byte[] newbuf = new byte[numBytes];
            System.arraycopy(data, 0, newbuf, 0, numBytes);
            data = newbuf;
         }
      }
      
      return data;
   }
   
   /**
    * Overridden to allow streaming data to be managed.
    * 
    * @return the byte read or -1 if end of the stream.
    * 
    * @throws IOException
    */
   @Override
   public int read() throws IOException   
   {
      int data = -1;
      
      byte[] buffer = new byte[1];
      int numBytes = read(buffer);
      while(numBytes != -1 && numBytes != 1)
      {
         numBytes = read(buffer);
      }
      
      if(numBytes != -1)
      {
         data = buffer[0] & 0xff;
      }
      
      return data;
   }
   
   /**
    * Overridden to allow streaming data to be managed.
    * 
    * @param b the buffer to read data into.
    * @param off the offset to start writing the read data at.
    * @param len the number of bytes to read.
    * 
    * @return the number of bytes read and modified.
    * 
    * @throws IOException
    */
   @Override
   public int read(byte[] b, int off, int len)
      throws IOException
   {
      // for storing managed data read from the underlying stream
      byte[] data = null;

      // for storing the number of bytes read from the processed data buffer
      int readBytes = 0;
      
      // keep reading until len is met
      while(len - readBytes != 0)
      {
         // if there is no processed data to read, do a managed read
         if(mProcessedData == null)
         {
            if(mManager != null)
            {
               data = performManagedRead(DEFAULT_READ_SIZE);
            }
            else
            {
               data = performUnmanagedRead(len);
            }
            
            // if there is no more data to process, break
            if(data == null)
            {
               break;
            }
            
            // if there is a processor
            if(mProcessor != null)
            {
               // see if this is the last chunk by checking to see if
               // end of stream reached and all data has passed through
               // data manager (or there isn't one)
               boolean last = (mEndOfStream && mReadPos == 0);
               mProcessedData = mProcessor.processStreamData(data, last);
            }
            else
            {
               mProcessedData = data;
            }

            // reset processed data read position
            mProcessedPos = 0;
         }
         
         // if there is no processed data
         if(mProcessedData == null)
         {
            // if end of the stream, break out, otherwise continue
            if(mEndOfStream)
            {
               break;
            }

            // continue through loop
            continue;
         }
         
         // figure out how many bytes to copy
         int remaining = len - readBytes;
         int amount = (remaining > mProcessedData.length - mProcessedPos) ?
               mProcessedData.length - mProcessedPos: remaining;
         
         // copy data from processed data buffer into user buffer
         System.arraycopy(mProcessedData, mProcessedPos, b, off, amount);
         
         // increment the processed data read position and read bytes
         mProcessedPos += amount;
         off += amount;
         readBytes += amount;
         
         // clear the processed data if it has all been read
         if(mProcessedPos == mProcessedData.length)
         {
            mProcessedData = null;
         }
      }

      // return -1 if no more data to read, otherwise return amount read
      return (mEndOfStream && (mProcessedData == null) && readBytes == 0) ?
             -1 : readBytes;
   }
   
   /**
    * Overridden to ensure that data is managed and
    * processed correctly when skipping.
    *
    * @param n the number of bytes to be skipped.
    * 
    * @return the actual number of bytes skipped. 
    */
   @Override
   public long skip(long n)
   {
      long count = 0;
      
      if(n != 0)
      {
         try
         {
            int numBytes = 0;
         
            int bufSize = 2048;
            byte[] buffer = new byte[bufSize];
            
            // read through stream until n reached, discarding data
            while(numBytes != -1 && count != n)
            {
               // get the number of bytes to read
               int readSize = Math.min((int)(n - count), bufSize);
               
               numBytes = read(buffer, 0, readSize);
               
               if(numBytes > 0)
               {                  
                  count += numBytes;
               }
               else
               {
                  mEndOfStream = true;
               }
            }
         }
         catch(Exception e)
         {
            LoggerManager.getLogger("dbstream").debug(getClass(), 
               LoggerManager.getStackTrace(e));
         }
      }
      
      return count;
   }
   
   /**
    * Sets the maximum number of bytes to read into the buffer at once.
    * 
    * @param maxReadSize the maximum number of bytes to read before
    *                    aborting. Set this to -1 for no max.
    */
   public void setMaxReadSize(int maxReadSize)
   {
      mMaxReadSize = maxReadSize;
   }
   
   /**
    * Gets the maximum number of bytes allowed to be read into the buffer
    * at once.
    * 
    * @return the maximum number of bytes allowed to be read before aborting.
    */
   public int getMaxReadSize()
   {
      return mMaxReadSize;
   }
   
   /**
    * Sets the stream manager for this managed input stream.
    * 
    * @param manager the manager to use with this stream.
    */
   public void setStreamManager(IStreamManager manager)
   {
      mManager = manager;
   }
   
   /**
    * Gets the stream manager for this managed input stream.
    * 
    * @return the manager for this stream.
    */
   public IStreamManager getStreamManager()
   {
      return mManager;
   }

   /**
    * Sets the stream processor for this managed input stream.
    * 
    * @param processor the processor to use with this stream.
    */
   public void setStreamProcessor(IStreamProcessor processor)
   {
      mProcessor = processor;
   }
   
   /**
    * Gets the stream processor for this managed input stream.
    * 
    * @return the processor for this stream.
    */
   public IStreamProcessor getStreamProcessor()
   {
      return mProcessor;
   }
}
