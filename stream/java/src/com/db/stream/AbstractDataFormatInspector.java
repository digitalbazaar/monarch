/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

/**
 * An AbstractDataFormatInspector inspects data by checking to see if its
 * format is recognized or not. Extending classes provide the actual
 * implementation details for detected particular data formats.
 * 
 * @author Dave Longley
 */
public abstract class AbstractDataFormatInspector
implements DataFormatInspector
{
   /**
    * A flag that indicates whether or not this inspector has inspected
    * enough data to determine if it recognizes its format.
    */
   protected boolean mDataSatisfied;
   
   /**
    * A flag that indicates whether or not this inspector recognizes the format
    * of the inspected data. 
    */
   protected boolean mFormatRecognized;
   
   /**
    * True if this inspector should continue to inspect data after being
    * satisfied, false if not.
    */
   protected boolean mInspectAfterSatisfied;
   
   /**
    * The total number of bytes inspected so far.
    */
   protected long mBytesInspected;
   
   /**
    * Stores the current number of bytes that the specific inspection
    * implementation can skip over.
    */
   protected long mSkipBytes;
   
   /**
    * Creates a new AbstractDataFormatInspector.
    */
   public AbstractDataFormatInspector()
   {
      mDataSatisfied = false;
      mFormatRecognized = false;
      mInspectAfterSatisfied = false;
      mBytesInspected = 0;
      mSkipBytes = 0;
   }
   
   /**
    * Called when enough data has been inspected for this inspector to
    * determine whether or not the format of the data is recognized.
    * 
    * @param recognized true if the data format is recognized, false if not.
    */
   protected void setFormatRecognized(boolean recognized)
   {
      mFormatRecognized = recognized;
      mDataSatisfied = true;
   }
   
   /**
    * Sets the current number of bytes that the inspection algorithm can skip
    * over. This is the number of bytes that will pass through this inspector
    * without calling detectFormat().
    * 
    * @param count the current number of bytes that the inspection algorithm
    *              can skip over.
    */
   protected void setSkipBytes(long count)
   {
      mSkipBytes = count;
   }
   
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
   public int inspectData(byte[] b, int offset, int length)
   {
      int rval = 0;
      
      // inspect data if not satisfied or if keep-inspecting enabled
      if(!isDataSatisfied() || keepInspecting())
      {
         // skip bytes as appropriate
         if(mSkipBytes > 0)
         {
            rval = Math.min((int)mSkipBytes, length);
            mSkipBytes -= rval;
         }
         else
         {
            // try to detect format
            rval = detectFormat(b, offset, length);
         }
         
         // increase number of inspected bytes
         mBytesInspected += rval;
      }
      else
      {
         // consider all data inspected
         rval = length;
      }
      
      return rval;
   }
   
   /**
    * Inspects the data in the passed buffer and tries to detect its
    * format. The number of bytes that were inspected is returned so that
    * they can be safely cleared from the passed buffer.
    * 
    * Subsequent calls to this method should be treated as if the data
    * in the passed buffer is consecutive in nature (i.e. read from a stream).
    * 
    * Once this inspector has determined that the inspected data is in
    * a known or unknown format, this inspector may opt to stop inspecting
    * data.
    * 
    * @param b the buffer with data to inspect.
    * @param offset the offset at which to begin inspecting bytes.
    * @param length the maximum number of bytes to inspect.
    * 
    * @return the number of bytes that were inspected in the passed buffer.
    */
   public abstract int detectFormat(byte[] b, int offset, int length);
   
   /**
    * Returns whether or not this inspector is "data-satisfied." The inspector
    * is data-satisfied when it has inspected enough data to determine whether
    * or not the format of the data is recognized.
    * 
    * @return true if this inspector has inspected enough data to determine
    *         if the format is or is not recognized, false if not.
    */
   public boolean isDataSatisfied()
   {
      return mDataSatisfied;
   }
   
   /**
    * Returns whether or not this inspector has recognized the format of
    * the data it has been inspecting.
    * 
    * @return true if the data format is recognized, false if not.
    */
   public boolean isFormatRecognized()
   {
      return mFormatRecognized;
   }
   
   /**
    * Sets whether or not this inspector should keep inspecting data after
    * being data-satisfied, false if not.
    * 
    * @param inspect true to keep inspecting, false not to.
    */
   public void setKeepInspecting(boolean inspect)
   {
      mInspectAfterSatisfied = inspect;
   }
   
   /**
    * Gets whether or not this inspector should keep inspecting data after
    * being data-satisfied, false if not.
    * 
    * @return true to keep inspecting, false not to.
    */
   public boolean keepInspecting()
   {
      return mInspectAfterSatisfied;
   }
   
   /**
    * Gets the total number of bytes inspected so far.
    * 
    * @return the number of bytes inspected so far.
    */
   public long getBytesInspected()
   {
      return mBytesInspected;
   }
   
   /**
    * Gets a string identifier for the format that was detected.
    * 
    * @return a string identifier for the format that was detected.
    */
   public abstract String getFormat();
   
   /**
    * Gets a custom readable report on the data inspection.
    * 
    * @return a custom readable report on the data inspection.
    */
   public abstract String getInspectionReport();
}
