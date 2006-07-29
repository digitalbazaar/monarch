/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

/**
 * A BandwidthThrottler is a class that is used to throttle bandwidth.
 * 
 * @author Dave Longley
 */
public class BandwidthThrottler
{
   /**
    * The rate limit for this BandwidthThrottler.
    */
   protected long mRateLimit;
   
   /**
    * The time at which a window began for requesting data.
    */
   protected long mWindowTime;
   
   /**
    * The number of bytes that have been granted in the current window.
    */
   protected long mBytesGranted;   
   
   /**
    * The amount of time (in milliseconds) that must pass before a byte
    * is available. This number is never more than 1000 and never less
    * than 1.
    */
   protected int mAvailableByteTime;
   
   /**
    * The number of available bytes.
    */
   protected long mAvailableBytes;
   
   /**
    * Creates a new BandwidthThrottler.
    * 
    * @param rateLimit the bytes/second rate limit to use. A value of 0
    *                  indicates no rate limit.
    */
   public BandwidthThrottler(int rateLimit)
   {
      // initialize the available number of bytes
      mAvailableBytes = 0;
      
      // set the rate limit (will also reset the window time if necessary)
      setRateLimit(rateLimit);
   }
   
   /**
    * Resets the window time.
    */
   protected void resetWindowTime()
   {
      // set the current window time
      mWindowTime = System.currentTimeMillis();
         
      // reset the bytes already granted in this window
      mBytesGranted = 0;
   }
   
   /**
    * Updates the time at which a window for requesting data began --
    * if the number of granted bytes in the current window is high enough.
    */
   protected void updateWindowTime()
   {
      // get the passed time in the current window
      long passedTime = System.currentTimeMillis() - getWindowTime();
      
      // cap the number of bytes granted per window at Integer.MAX_VALUE
      // so that there isn't any overflow -- this should also be a
      // sufficiently large enough number such that rate calculations
      // aren't affected very often at all
      if(mBytesGranted > Integer.MAX_VALUE)
      {
         resetWindowTime();
      }
      else if(passedTime > 60000)
      {
         // cap passed time at 1 minute to avoid overflow
         resetWindowTime();
      }
   }
   
   /**
    * Gets the time at which the current window for requesting
    * data began. 
    * 
    * @return the time at which the current window for requesting data began.
    */
   protected long getWindowTime()
   {
      return mWindowTime;
   }
   
   /**
    * Updates the amount of time (in milliseconds) that must pass before
    * a byte is available.
    */
   protected void updateAvailableByteTime()
   {
      // the amount of time until a byte is available is 1000 milliseconds
      // divided by the rate in bytes/second, with a minimum of 1 millisecond
      mAvailableByteTime = (int)Math.round(1000D / getRateLimit());
      mAvailableByteTime = Math.max(1, mAvailableByteTime);
   }
   
   /**
    * Gets the amount of time (in milliseconds) that must pass before
    * a byte is available. This number is never more than 1000 and never
    * less than 1.
    * 
    * @return the amount of time (in milliseconds) that must pass before
    *         a byte is available.
    */
   protected int getAvailableByteTime()
   {
      return mAvailableByteTime;
   }
   
   /**
    * Updates the number of bytes that are currently available.
    */
   protected void updateAvailableBytes()
   {
      // get the passed time in the current window
      double passedTime = System.currentTimeMillis() - getWindowTime();
      
      // determine how many bytes are available given the passed time --
      // use the floor so as not to go over the rate limit
      mAvailableBytes = (long)Math.floor(passedTime / 1000D * getRateLimit());
      
      // subtract the number of bytes already granted in this window
      mAvailableBytes -= mBytesGranted;
      mAvailableBytes = Math.max(0, mAvailableBytes);
   }
   
   /**
    * Gets the number of bytes that are currently available.
    * 
    * @return the number of bytes that are currently available.
    */
   protected long getAvailableBytes()
   {
      return mAvailableBytes;
   }
   
   /**
    * This method blocks until at least one byte is available without
    * violating the rate limit or until the current thread has been
    * interrupted.
    * 
    * @throws InterruptedException
    */
   protected void limitBandwidth() throws InterruptedException
   {
      // update the window time
      updateWindowTime();

      // update the number of available bytes
      updateAvailableBytes();
      
      // while there aren't any available bytes, sleep for the
      // available byte time
      while(getAvailableBytes() == 0)
      {
         Thread.sleep(getAvailableByteTime());
         
         // update the number of available bytes
         updateAvailableBytes();
      }
   }

   /**
    * Requests the passed number of bytes from this throttler. This method
    * will block until at least one byte can be sent without violating
    * the rate limit or if the current thread has been interrupted.
    * 
    * @param count the number of bytes requested.
    * 
    * @return the number of bytes permitted to send.
    * 
    * @throws InterruptedException
    */
   public synchronized int requestBytes(int count) throws InterruptedException
   {
      int rval = 0;
      
      if(getRateLimit() > 0)
      {
         // limit the bandwidth
         limitBandwidth();
         
         // get the available bytes
         rval = Math.min((int)getAvailableBytes(), count);
         
         // increment the bytes granted
         mBytesGranted += rval;
      }
      else
      {
         // no rate limit, return the count
         rval = count;
      }
      
      return rval;
   }

   /**
    * Sets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    * 
    * @param rateLimit the bytes/second rate limit to use.
    */
   public synchronized void setRateLimit(long rateLimit)
   {
      // set new rate limit
      mRateLimit = Math.max(0, rateLimit);
      
      if(mRateLimit > 0)
      {
         // reset the window time
         resetWindowTime();
         
         // update the available byte time
         updateAvailableByteTime();
      }
   }
   
   /**
    * Gets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    * 
    * @return the rate limit in bytes/second.
    */
   public long getRateLimit()
   {
      return mRateLimit;
   }
}
