/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PushbackInputStream;

/**
 * A PipeInputStream is a stream that reads from some filtered internal stream,
 * writes the data to some output stream, and then finally reads
 * data from wherever that data was written to. 
 * 
 * This class maintains an internal output stream that is used to populate
 * an internal read buffer for reading. The output stream is written to
 * whenever a read() is called on this stream. The output stream receives
 * its data from the input stream that is filtered through this pipe.
 * 
 * The internal read buffer will be the final source for all read() calls.
 * 
 * So, some input stream is assigned to this pipe. When read() is called on
 * this pipe, that input stream is read from and its data is written to
 * an internal read buffer by some output stream. The data in the internal
 * read buffer is then read and returned in the read() call. This allows
 * output streams to modify data that is read in by the internal read stream.
 * 
 * An example usage of this class is below:
 * 
 * Suppose we want to read data from some input stream and gzip it as it is
 * streamed. In order to do this, we need to read from the input stream and
 * write the gzipped data out to some output stream. However, we want to read
 * the data that is output back in via an input stream. This class allows
 * this kind of behavior by piping the output from the output stream back
 * into an internal buffer for reading.
 * 
 * // first create the pipe input stream and give it the original
 * // input stream to read from
 * PipeInputStream pipe = new PipeInputStream(inputStream);
 * 
 * // next create the gzip output stream and tell it to write to the
 * // pipe input stream's internal output stream
 * GZIPOutputStream gzos = new GZIPOutputStream(pipe.getOutputStream());
 * 
 * // next set the gzip output stream as the internal output stream
 * pipe.setOutputStream(gzos);
 * 
 * // read away
 * pipe.read();
 * 
 * @author Dave Longley
 */
public class PipeInputStream extends PushbackInputStream
{
   /**
    * The internal read buffer.
    */
   protected byte[] mReadBuffer;
   
   /**
    * The current read position in the read buffer.
    */
   protected int mReadPos;
   
   /**
    * The internal byte array output stream for this pipe input stream.
    */
   protected ByteArrayOutputStream mByteArrayOutputStream;
   
   /**
    * The internal output stream used to write to the byte array
    * output stream.
    */
   protected OutputStream mOutputStream;
   
   /**
    * Set to true when the internal output stream has been closed,
    * false while it has not been closed.
    */
   protected boolean mOutputStreamClosed;
   
   /**
    * An unread buffer.
    */
   protected byte[] mUnreadBuffer;
   
   /**
    * The current unread position.
    */
   protected int mUnreadPos;
   
   /**
    * Creates a new pipe input stream that reads data from the passed
    * input stream.
    * 
    * @param is the input stream to read data from.
    */
   public PipeInputStream(InputStream is)
   {
      this(is, 0);
   }

   /**
    * Creates a new pipe input stream that reads data from the passed
    * input stream.
    * 
    * @param is the input stream to read data from.
    * @param unreadSize the unread buffer size.
    */
   public PipeInputStream(InputStream is, int unreadSize)
   {
      super(is);
      
      // set up read buffer
      mReadBuffer = new byte[0];
      mReadPos = 0;
      
      // create byte array output stream
      mByteArrayOutputStream = new ByteArrayOutputStream();
      
      // set internal output stream
      mOutputStream = mByteArrayOutputStream;
      
      // output stream not yet closed
      mOutputStreamClosed = false;
      
      // set up unread buffer
      mUnreadBuffer = new byte[unreadSize];
      mUnreadPos = 0;
   }
   
   /**
    * Fills the read buffer as appropriate.
    * 
    * @return the new size of the read buffer or -1 if there is no more
    *         data to be read.
    * 
    * @throws IOException
    */
   protected int fillReadBuffer() throws IOException
   {
      int rval = mReadBuffer.length;
      
      if(mReadPos == mReadBuffer.length)
      {
         rval = -1;
         
         if(!mOutputStreamClosed)
         {
            // read some data from the internal input stream
            byte[] buffer = new byte[16384];
            int numBytes = in.read(buffer, 0, buffer.length);
            if(numBytes != -1)
            {
               // write data to internal output stream
               mOutputStream.write(buffer, 0, numBytes);
               mOutputStream.flush();
            }
            else
            {
               // close the output stream
               mOutputStream.close();
               mOutputStreamClosed = true;
            }
            
            // get a new read buffer
            mReadBuffer = mByteArrayOutputStream.toByteArray();
            rval = mReadBuffer.length;
            
            // reset byte array output stream
            mByteArrayOutputStream.reset();
            
            // reset read position
            mReadPos = 0;
         }
      }
      
      return rval;
   }
   
   /**
    * Reads the next byte of data from this input stream. The value 
    * byte is returned as an <code>int</code> in the range 
    * <code>0</code> to <code>255</code>. If no byte is available 
    * because the end of the stream has been reached, the value 
    * <code>-1</code> is returned. This method blocks until input data 
    * is available, the end of the stream is detected, or an exception 
    * is thrown. 
    *
    * @return the next byte of data, or <code>-1</code> if the end of the
    *         stream is reached.
    * @throws IOException
    */
   public int read() throws IOException
   {
      int b = -1;
      
      // try to read from the unread buffer
      if(mUnreadPos > 0)
      {
         b = mUnreadBuffer[--mUnreadPos];
      }
      else
      {
         // fill the read buffer as appropriate
         if(fillReadBuffer() != -1)
         {
            // get the next byte in the read buffer and increment read position
            b = mReadBuffer[mReadPos];
            mReadPos++;
         }
      }
      
      return b;
   }

   /**
    * Reads up to <code>len</code> bytes of data from this input stream 
    * into an array of bytes. This method blocks until some input is 
    * available. 
    *
    * @param buffer the buffer into which the data is read.
    * @param offset the start offset of the data.
    * @param length the maximum number of bytes read.
    * @return the total number of bytes read into the buffer, or
    *         <code>-1</code> if there is no more data because the end of
    *         the stream has been reached.
    * @throws IOException
    */
   public int read(byte buffer[], int offset, int length) throws IOException
   {
      int numBytes = -1;
      
      // try to read from the unread buffer
      if(mUnreadPos > 0)
      {
         // read from the end of the unread buffer
         int count = Math.min(mUnreadPos, length);
         System.arraycopy(mUnreadBuffer, mUnreadPos - count,
                          buffer, offset, count);
         
         // decrease unread position by amount read
         mUnreadPos -= count;
         
         // read again if not enough data was read 
         if(count < length)
         {
            numBytes = read(buffer, offset + count, length - count);
            if(numBytes > 0)
            {
               numBytes += count;
            }
            else
            {
               numBytes = count;
            }
         }
         else
         {
            numBytes = count;
         }
      }
      else
      {
         // fill the read buffer as appropriate
         if(fillReadBuffer() != -1)
         {
            int amount = Math.min(mReadBuffer.length - mReadPos, length);
            
            // copy data from internal read buffer into passed buffer
            System.arraycopy(mReadBuffer, mReadPos, buffer, offset, amount);
            
            // increment read position
            mReadPos += amount;
            
            // set number of bytes read
            numBytes = amount;
         }
      }
      
      return numBytes;
   }
   
   /**
    * Unreads some data so that it can be read again by this stream.
    * 
    * @param b the byte to unread.
    */
   public void unread(int b)
   {
      byte[] buffer = new byte[1];
      buffer[0] = (byte)b;
      unread(buffer, 0, buffer.length);
   }

   /**
    * Unreads some data so that it can be read again by this stream.
    * 
    * @param buffer the buffer of data to unread back onto the stream.
    * @param offset the offset to start unreading in the passed buffer.
    * @param length the number of bytes to unread.
    */
   public void unread(byte[] buffer, int offset, int length)
   {
      if((mUnreadPos + length) > mUnreadBuffer.length)
      {
         // resize unread buffer as necessary
         byte[] newBuffer = new byte[mUnreadPos + length];
         System.arraycopy(mUnreadBuffer, 0, newBuffer, 0, mUnreadPos);
         mUnreadBuffer = newBuffer;
      }
      
      // unread into the unread buffer
      System.arraycopy(buffer, offset, mUnreadBuffer, mUnreadPos, length);
      mUnreadPos += length;
   }
   
   /**
    * Returns the number of bytes that can be read from this input stream
    * without blocking.
    *
    * @return the number of bytes that can be read from the input stream
    *         without blocking.
    * @throws IOException
    */
   public int available() throws IOException    
   {
      int available = mUnreadPos;
      
      available += (mReadBuffer.length - mReadPos);
      
      return available;
   }

   /**
    * Skips over and discards <code>n</code> bytes of data from the 
    * input stream. The <code>skip</code> method may, for a variety of 
    * reasons, end up skipping over some smaller number of bytes, 
    * possibly <code>0</code>. The actual number of bytes skipped is 
    * returned.
    *
    * @param n the number of bytes to be skipped.
    * @return the actual number of bytes skipped.
    * @throws IOException
    */
   public long skip(long n) throws IOException
   {
      long skipped = 0;
      
      // create a buffer for reading
      byte[] buffer = new byte[65536];
      
      // read and throw away
      int numBytes = 0;
      while(n > 0 && numBytes != -1)
      {
         int count = (int)Math.min(buffer.length, n);
         numBytes = read(buffer, 0, count);
         
         if(numBytes != -1)
         {
            n -= numBytes;
            skipped += numBytes;
         }
      }
      
      return skipped;
   }

   /**
    * Closes this input stream and releases any system resources 
    * associated with the stream.
    * 
    * This method will close the internal input stream and
    * internal output stream. 
    *
    * @throws IOException
    */
   public void close() throws IOException
   {
      super.close();
      mOutputStream.close();
      mReadBuffer = null;
      mUnreadBuffer = null;
   }

   /**
    * This stream does not support the mark method.
    *
    * @param readlimit the maximum limit of bytes that can be read
    *                  before the mark position becomes invalid.
    */
   public synchronized void mark(int readlimit)
   {
      // do nothing
   }

   /**
    * This stream does not support the reset method.
    * 
    * @throws IOException
    */
   public synchronized void reset() throws IOException
   {
      // do nothing
   }

   /**
    * Returns false. This stream does not support the mark and reset methods.
    *
    * @return false, this stream does not support the mark and set methods.
    */
   public boolean markSupported()
   {
      return false;
   }
   
   /**
    * Sets the output stream for this pipe input stream. This is
    * the stream that is used to fill the internal read buffer. The passed
    * stream should wrap the original output stream for this pipe input
    * stream.
    *  
    * @param os the output stream used to fill the internal buffer
    *           that is read from when read() is called on this stream.
    */
   public void setOutputStream(OutputStream os)
   {
      mOutputStream = os;
   }
   
   /**
    * Gets the output stream for this pipe input stream. This is
    * the stream that is used to fill the internal read buffer that is
    * read from when read() is called on this stream.
    * 
    * @return the output stream used to fill the internal buffer
    *         that is read from when read() is called on this stream.
    */
   public OutputStream getOutputStream()
   {
      return mOutputStream;
   }
}
