/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

/**
 * Manages a stream of data by detecting its format and parsing the data
 * into that appropriate format.
 * 
 * @author Dave Longley
 */
public abstract class StreamFormatDetector implements IStreamManager
{
   /**
    * Set to true when this detector has determined that it
    * recognizes the format of the stream data. This is not set for
    * each chunk of data that is inspected, it is set once the detector
    * has determined that the entire stream of data could be recognized
    * according to a known format.
    */
   protected boolean mFormatDetected;
   
   /**
    * Set to true when this detector should continue trying to detect
    * the streaming data even after it has decided whether or not the data
    * matches a known format. 
    */
   protected boolean mKeepDetecting;
   
   /**
    * The maximum size, total, to read before deciding that the
    * data isn't in a known format.
    */
   protected long mMaxReadSize;
   
   /**
    * The total amount of data allowed to pass through this detector
    * so far.
    */
   protected long mBytesRead;
   
   /**
    * The current skip count. This is the current amount of bytes to skip
    * over until the next call to detectFormat.
    */
   protected long mSkipCount;
   
   /**
    * The current number of bytes skipped over. This number will reset to
    * zero when the current skip count is met.
    */
   protected long mSkippedBytes;
   
   /**
    * Creates a new format detector.
    */
   public StreamFormatDetector()
   {
      mFormatDetected = false;
      mKeepDetecting = true;
      mMaxReadSize = -1;
      mBytesRead = 0;
      mSkipCount = 0;
      mSkippedBytes = 0;
   }
   
   /**
    * Set to true when this detector has determined that it
    * recognizes the format of the stream data. This is not set for
    * each chunk of data that is inspected, it is set once the detector
    * has determined that the entire stream of data could be recognized
    * according to a known format.
    *
    * @param detected true to indicate the detector has determined the
    *                 entire stream of data could be classified by
    *                 a recognized format, false otherwise.
    */
   protected void setFormatDetected(boolean detected)
   {
      mFormatDetected = detected;
   }
   
   /**
    * Gets whether or not the detector should keep trying to detect the
    * data even after the format of the streaming data has been successfully
    * detected. 
    * 
    * @return true if the detector should keep detecting, false if not.
    */
   protected boolean keepDetecting()
   {
      return mKeepDetecting;
   }
   
   /**
    * Sets the total number of bytes read so far.
    * 
    * @param bytesRead the number of bytes read so far.
    */
   protected void setBytesRead(long bytesRead)
   {
      mBytesRead = bytesRead;
   }
   
   /**
    * Gets the total number of bytes read so far.
    * 
    * @return the number of bytes read so far.
    */
   protected long getBytesRead()
   {
      return mBytesRead;
   }
   
   /**
    * Sets the skip count. This is the number of bytes to skip
    * over until checking data with detectFormat again.
    * 
    * @param count the skip count to set.
    */
   protected void setSkipCount(long count)
   {
      mSkipCount = count;
      setSkippedBytes(0);
   }
   
   /**
    * Gets the skip count. This is the number of bytes to skip
    * over until checking data with detect format again.
    * 
    * @return the skip count.
    */
   protected long getSkipCount()
   {
      return mSkipCount;
   }
   
   /**
    * Sets the number of bytes skipped so far. This number will be
    * reset to zero when it reaches the current skip count.
    * 
    * @param skipped the number of bytes skipped so far.
    */
   protected void setSkippedBytes(long skipped)
   {
      mSkippedBytes = skipped;
   }
   
   /**
    * Gets the number of bytes skipped so far. This number will be
    * reset to zero when it reaches the current skip count.
    * 
    * @return the number of bytes skipped so far.
    */
   protected long getSkippedBytes()
   {
      return mSkippedBytes;
   }
   
   /**
    * Convenience method. Returns true if the max read limit has been
    * exceeded, false if not.
    * 
    * @return true if read limit exceeded, false if not.
    */
   public boolean readLimitExceeded()
   {
      boolean rval = false;
      
      if(getMaxReadSize() != -1 && getBytesRead() > getMaxReadSize())
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns whether or not the detector has determined that it
    * recognizes the format of the stream data.
    * 
    * @return true if the format is recognized, false if not.
    */
   public boolean formatDetected()
   {
      return mFormatDetected;
   }
   
   /**
    * Sets whether or not the detector should keep detecting the data even
    * after it has decided whether or not the data matches a known format.
    * 
    * @param detect if set to true, the detector will always continue to detect
    *               the data, attempting to parse it into a known format,
    *               if false, the manager will simply let the data pass
    *               through undetected.
    */
   public void setKeepDetecting(boolean detect)
   {
      mKeepDetecting = detect;
   }
   
   /**
    * Sets the maximum read size.
    * 
    * @param readSize the maximum read size.
    */
   public void setMaxReadSize(long readSize)
   {
      mMaxReadSize = readSize;
   }
   
   /**
    * Gets the maximum read size.
    * 
    * @return the maximum read size.
    */
   public long getMaxReadSize()
   {
      return mMaxReadSize;
   }
   
   /**
    * Checks the passed buffer to see if it is ready to be
    * processed, if it isn't it returns the number of
    * additional bytes needed. If it is ready to be
    * processed, the number of extra bytes that shouldn't
    * be yet processed is returned (zero or a negative number).
    * 
    * @param data the data buffer with the data that has been read so far.
    * @param offset the offset for valid data in the buffer.
    * @param length the number of bytes that are valid.
    * @return the positive number of additional bytes required before
    *         the data can be processed, or the negative number of
    *         extra bytes to be saved until this method requests them, or
    *         zero indicating that the data is ready to be processed.
    */
   public int manageStreamData(byte[] data, int offset, int length)   
   {
      int requiredBytes = 0;
      
      // if keep detecting is set or the format hasn't been detected yet
      if(keepDetecting() || !formatDetected())
      {
         // if keep detecting or read limit not exceeded
         // then try to detect format
         if(keepDetecting() || !readLimitExceeded())
         {
            // if skip count is set, continue to skip data until skip is met
            if(getSkipCount() > 0)
            {
               // determine bytes to skip
               long skip = getSkippedBytes() + length;
               
               // correct required bytes if skip greater than skip count
               if(skip > getSkipCount())
               {
                  requiredBytes = (int)(getSkipCount() - skip);
                  skip = length + requiredBytes;
                  setSkipCount(0);
               }
               else if(skip == getSkipCount())
               {
                  setSkipCount(0);
               }
               else
               {
                  // update skipped bytes
                  setSkippedBytes(skip);
               }
               
               // update bytes read?
               // don't update bytes read if only bytes that have
               // passed through detectFormat should be counted
               // which is probably the desired behavior
               //setBytesRead(getBytesRead() + skip);
            }
            else
            {
               // try to detect the format
               requiredBytes = detectFormat(data, offset, length);
            
               // if format of chunk was detected
               if(requiredBytes <= 0)
               {
                  // set format flag if stream data format detected
                  if(!getStreamFormat().equals(""))
                  {
                     setFormatDetected(true);
                  }
                  
                  // increment bytes read (that have passed through manager)
                  // add required bytes because they will be negative or zero
                  setBytesRead(getBytesRead() + length + requiredBytes);
               }
               else if(!keepDetecting() && getMaxReadSize() != -1 &&
                       length > getMaxReadSize())
               {
                  // max limit exceeded, increment bytes read by data length 
                  setBytesRead(getBytesRead() + length);
               }
            }
         }
      }
      
      return requiredBytes;
   }   
   
   /**
    * Attempts to detect the format of the data that is passed. If the
    * format could not be detected, it returns the number of bytes that
    * should be necessary to detect the format. If it successfully
    * detects the format and the data was exactly the right size for
    * detecting the format, it returns zero. If there is more than enough
    * data to determine the format, it returns the number of extra bytes
    * that are not necessary to determine the format so they can be used
    * in subsequent detections.
    * 
    * @param data the data buffer with the that has been read so far.
    * @param offset the offset for valid data in the buffer.
    * @param length the number of valid bytes in the data buffer.
    * @return the positive number of additional bytes required before
    *         the data's format can be detected, or the negative number of
    *         extra bytes to be saved until this method requests them, or
    *         zero indicating that the data's length is perfect for
    *         detecting the format.
    */
   public abstract int detectFormat(byte[] data, int offset, int length);
   
   /**
    * Returns the stream's detected format, if it was detected. If not,
    * a blank string is returned.
    * 
    * @return the stream's detected format or a blank string if not detected.
    */
   public abstract String getStreamFormat();
   
   /**
    * Returns a readable report on the detection.
    * 
    * @return a readable report on the detection.
    */
   public abstract String getDetectionReport();
}
