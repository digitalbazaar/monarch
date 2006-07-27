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
    * The amount of time (in milliseconds) that must pass before a byte
    * is available.
    */
   protected long mAvailableByteTime;
   
   /**
    * The number of available bytes.
    */
   protected int mAvailableBytes;
   
   /**
    * The number of bytes that have been permitted in the current window.
    */
   protected int mBytesPermitted;
   
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
      
      // set the rate limit
      setRateLimit(rateLimit);
   }
   
   /**
    * Updates the time at which a window for requesting data began --
    * if the last request time was long enough ago.
    */
   protected synchronized void updateWindowTime()
   {
      // set the current window time
      mWindowTime = System.currentTimeMillis();
      
      // reset the bytes permitted in this window
      mBytesPermitted = 0;
   }
   
   /**
    * Gets the time at which the current window for requesting
    * data began. 
    * 
    * @return the time at which the current window for requesting data began.
    */
   protected synchronized long getWindowTime()
   {
      return mWindowTime;
   }
   
   /**
    * Updates the amount of time (in milliseconds) that must pass before
    * a byte is available.
    */
   protected synchronized void updateAvailableByteTime()
   {
      // the amount of time until a byte is available is 1000 milliseconds
      // divided by the rate in bytes/second, with a minimum of 1 millisecond
      mAvailableByteTime = (int)Math.round(1000D / getRateLimit());
      mAvailableByteTime = Math.max(1, mAvailableByteTime);
   }
   
   /**
    * Gets the amount of time (in milliseconds) that must pass before
    * a byte is available.
    * 
    * @return the amount of time (in milliseconds) that must pass before
    *         a byte is available.
    */
   protected synchronized long getAvailableByteTime()
   {
      return mAvailableByteTime;
   }
   
   /**
    * Updates the number of bytes that are currently available.
    */
   protected synchronized void updateAvailableBytes()
   {
      // get the passed time in the current window
      double passedTime = System.currentTimeMillis() - getWindowTime();
      
      // determine how many bytes are available given the passed time
      long bytes = Math.round(passedTime / 1000D * getRateLimit());
      mAvailableBytes = (int)Math.min(Integer.MAX_VALUE, bytes);
      
      // subtract the number of bytes already permitted in this window
      mAvailableBytes -= mBytesPermitted;
      mAvailableBytes = Math.max(0, mAvailableBytes);
   }
   
   /**
    * Gets the number of bytes that are currently available.
    * 
    * @return the number of bytes that are currently available.
    */
   protected synchronized int getAvailableBytes()
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
   protected synchronized void limitBandwidth() throws InterruptedException
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
         rval = Math.min(getAvailableBytes(), count);
         
         // increment the bytes permitted
         mBytesPermitted += rval;
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
         // update the available byte time
         updateAvailableByteTime();
      }
   }
   
   /**
    * Gets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    * 
    * @return the rate limit in bytes/second.
    */
   public synchronized long getRateLimit()
   {
      return mRateLimit;
   }
}
