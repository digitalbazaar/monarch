/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

import java.io.FilterInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.Vector;

/**
 * This class is used to read from an input stream until a boundary
 * is reached.
 * 
 * @author Dave Longley
 */
public class BoundaryInputStream extends FilterInputStream
{
   /**
    * A buffer for valid bytes that are not part of a boundary.
    */
   protected byte[] mReadBuffer;
   
   /**
    * The read buffer position. This is the position of the last
    * valid byte in the read buffer.
    */
   protected int mReadBufferPos;
   
   /**
    * The current read position. This is the index for the next
    * byte to be read from the read buffer by a user.
    */
   protected int mReadPos;

   /**
    * The boundary buffer. Used to store data previously read and
    * suspected to be a boundary.
    */
   protected byte[] mBoundaryBuffer;
   
   /**
    * The boundary buffer position. This is the index of the last
    * valid byte in the boundary buffer.
    */
   protected int mBoundaryBufferPos;
   
   /**
    * A vector of boundaries.
    */
   protected Vector mBoundaries;
   
   /**
    * Whether or not a boundary has been reached.
    */
   protected boolean mBoundaryReached;
   
   /**
    * Gets the boundary that was reached.
    */
   protected String mBoundary;
   
   /**
    * The length of the longest boundary.
    */
   protected int mMaxBoundaryLength;
   
   /**
    * The length of the shortest boundary.
    */
   protected int mMinBoundaryLength;
   
   /**
    * Creates a new boundary input stream.
    * 
    * @param is the input stream to read from. 
    */
   public BoundaryInputStream(InputStream is)
   {
      super(is);
      
      mReadBuffer = new byte[2048];
      mReadBufferPos = 0;
      mReadPos = 0;
      
      mBoundaryBuffer = new byte[1];
      mBoundaryBufferPos = 0;

      mBoundaries = new Vector();
      mBoundaryReached = false;
      mMaxBoundaryLength = 1;
      mMinBoundaryLength = 1;
   }
   
   /**
    * Gets the underlying input stream.
    * 
    * @return the underlying input stream.
    */
   protected InputStream getInputStream()
   {
      return this.in;
   }
   
   /**
    * Sets the underlying input stream.
    * 
    * @param is the input stream to set as the underlying input stream.
    */
   protected void setInputStream(InputStream is)
   {
      this.in = is;
   }
   
   /**
    * Resizes the boundary buffer to the maximum boundary length and
    * ensures the read buffer is larger than the boundary buffer.
    */
   protected void resizeBuffers()
   {
      if(mMaxBoundaryLength != mBoundaryBuffer.length)
      {
         byte[] buffer = new byte[mMaxBoundaryLength];
         int length = Math.min(mBoundaryBufferPos, mMaxBoundaryLength);
         System.arraycopy(mBoundaryBuffer, 0, buffer, 0, length);
         mBoundaryBuffer = buffer;
      }
      
      if(mBoundaryBuffer.length >= mReadBuffer.length)
      {
         byte[] buffer = new byte[mReadBuffer.length + mBoundaryBuffer.length];
         System.arraycopy(mReadBuffer, 0, buffer, 0, mReadBufferPos);
         mReadBuffer = buffer;
      }
   }
   
   /**
    * Dumps the boundary buffer into the read buffer after determining
    * that it does not hold a true boundary.
    */
   protected void dumpBoundaryBuffer()
   {
      if(mBoundaryBufferPos != 0)
      {
         // no match was found, so dump the boundary buffer into
         // the read buffer and reset its position
         System.arraycopy(mBoundaryBuffer, 0,
                          mReadBuffer, mReadBufferPos, mBoundaryBufferPos);
         
         // increase the read buffer position and
         // reset boundary buffer position
         mReadBufferPos += mBoundaryBufferPos;
         mBoundaryBufferPos = 0;
      }      
   }
   
   /**
    * Gets all possible matching boundary characters. These are characters
    * that could result in a matching boundary.
    * 
    * @return the possible matching boundary characters.
    */
   protected char[] getMatchingBoundaryChars()
   {
      // get all boundaries that are long enough
      Vector v = new Vector();
      for(int i = 0; i < mBoundaries.size(); i++)
      {
         String boundary = (String)mBoundaries.get(i);
         if(boundary.length() > mBoundaryBufferPos)
         {
            v.add(boundary);
         }
      }
      
      // get potential matches
      char[] matches = new char[v.size()];
      for(int i = 0; i < v.size(); i++)
      {
         String boundary = (String)v.get(i);
         matches[i] = boundary.charAt(mBoundaryBufferPos);
      }
      
      return matches;
   }
   
   /**
    * Checks the boundary buffer to see if a boundary has been reached.
    * 
    * @return true if the boundary has been reached, false if not.
    */
   protected boolean boundaryReached()
   {
      // only check if not checked previously and minimum reached
      if(!mBoundaryReached && mBoundaryBufferPos >= mMinBoundaryLength)
      {
         Iterator i = mBoundaries.iterator();
         while(i.hasNext())
         {
            String boundary = (String)i.next();
            
            // get all the boundaries that have matching lengths
            if(boundary.length() == mBoundaryBufferPos)
            {
               // compare the boundary buffer to the boundary
               String bbuf =
                  new String(mBoundaryBuffer, 0, mBoundaryBufferPos);
               
               if(boundary.equals(bbuf))
               {
                  getLogger().debug("BOUNDARY REACHED!: " + boundary);

                  // set boundary reached
                  mBoundary = boundary;
                  mBoundaryReached = true;
                  break;
               }
            }
         }
      }
      
      return mBoundaryReached;
   }
   
   /**
    * Fills the read buffer.
    * 
    * @return true if the read buffer was filled, false if not, and the
    *         boundary or end of the stream has been reached.
    * @throws IOException 
    */
   protected boolean fillReadBuffer() throws IOException
   {
      boolean rval = false;
      
      // reset read position and buffer position
      mReadPos = 0;
      mReadBufferPos = 0; 
      
      // make sure the boundary has not yet been reached
      if(!boundaryReached())
      {
         // the max read size is the read buffer size minus the boundary
         // buffer size because the boundary buffer might need to be
         // dumped into the read buffer at any time
         int maxReadSize = mReadBuffer.length - mBoundaryBuffer.length;
         
         // read data from the input stream
         InputStream is = getInputStream();
         int data = -1;
         while(!boundaryReached() && mReadBufferPos < maxReadSize &&
               (data = is.read()) != -1)
         {
            // get the byte
            byte b = (byte)data;

            // cast data to a character
            char c = (char)data; 
            
            // whether or not a match was found
            boolean matchFound = false;
            
            // get the possible boundary characters
            char[] matches = getMatchingBoundaryChars();
            for(int i = 0; i < matches.length; i++)
            {
               // if found a match
               if(c == matches[i])
               {
                  // add the byte to the boundary buffer
                  mBoundaryBuffer[mBoundaryBufferPos] = b;
                  mBoundaryBufferPos++;
                  matchFound = true;
                  break;
               }
            }
            
            // if a match was not found 
            if(!matchFound)
            {
               // dump boundary buffer into read buffer
               dumpBoundaryBuffer();

               // add the read byte to the read buffer
               mReadBuffer[mReadBufferPos] = b;
               mReadBufferPos++;
            }
         }
         
         // if there is data in the read buffer, then return true
         if(mReadBufferPos > 0)
         {
            rval = true;
         }
      }
      
      return rval;
   }

   /**
    * Adds a new boundary.
    * 
    * @param boundary the boundary to add.
    */
   public void addBoundary(String boundary)
   {
      mBoundaries.add(boundary);
      
      mMinBoundaryLength = boundary.length();
      
      Iterator i = mBoundaries.iterator();
      while(i.hasNext())
      {
         String b = (String)i.next();
         
         // update max boundary length
         if(b.length() > mMaxBoundaryLength) 
         {
            mMaxBoundaryLength = b.length();
         }
         
         // update min boundary length
         if(b.length() < mMinBoundaryLength)
         {
            mMinBoundaryLength = b.length();
         }
      }
      
      // make sure the buffers are the correct size
      resizeBuffers();         
   }
   
   /**
    * Removes a boundary.
    *
    * @param boundary the boundary to remove. 
    */
   public void removeBoundary(String boundary)
   {
      mBoundaries.remove(boundary);
   }
   
   /**
    * Reads the next byte of data available in this stream. If a
    * boundary or the end of the stream is reached, this method will
    * return -1. Otherwise it will return the byte read as an integer.
    * 
    * @return the byte read or -1 if a boundary or the end of the 
    *         stream has been reached.
    * @throws IOException
    */
   public int read() throws IOException
   {
      int data = -1;
      
      // see if the whole read buffer has been read
      if(mReadPos == mReadBufferPos)
      {
         // try to fill the read buffer
         if(fillReadBuffer())
         {
            // read buffer filled, get data
            data = mReadBuffer[mReadPos] & 0xff;
            mReadPos++;
         }
      }
      else
      {
         data = mReadBuffer[mReadPos] & 0xff;
         mReadPos++;
      }
      
      return data;
   }
   
   /**
    * Reads up to <code>byte.length</code> bytes of data from this
    * input stream. If a boundary or the end of the stream is reached,
    * this method will return -1. Otherwise it will return the number of
    * bytes read.
    * 
    * @param b the buffer to read with.
    * @param off the offset to start reading at.
    * @param len the maximum number of bytes to read.
    * @return the number of bytes read or -1 if a boundary or the end of the 
    *         stream has been reached.
    * @throws IOException
    */   
   public int read(byte b[], int off, int len) throws IOException
   {
      int numBytes = -1;
      
      for(int i = 0; i < len; i++)
      {
         int data = read();
         
         if(data == -1)
         {
            break;
         }

         b[off + i] = (byte)data;
         numBytes = (i + 1);
      }
      
      if(numBytes == -1)
      {
         getLogger().debug("END OF BOUNDARY STREAM");
      }
      
      return numBytes;
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
      long skipped = -1;
      
      // skip n bytes, or until boundary/end of stream reached
      for(long i = 0; i < n; i++)
      {
         int data = read();
         if(data == -1)
         {
            // boundary or end of stream reached, break out
            break;
         }

         // increment amount skipped
         skipped = (i + 1);
      }
      
      return skipped;
   }

   /**
    * Gets the boundary that was reached or null if one wasn't.
    * 
    * @return the boundary that was reached or null if one wasn't.
    */
   public String getReachedBoundary()
   {
      return mBoundary;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
