/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A RateAverager is a utility class that is used to produce an accurate
 * average rate over a period of time. It uses a sliding time window overwhich
 * it measures the rate at which items have passed through this RateAverager.
 * 
 * The total number of items that have passed through this RateAverager
 * during a window of time divided by the length of the time window 
 * produces the average rate. 
 * 
 * @author Dave Longley
 */
public class RateAverager
{
   /**
    * Whether or not this RateAverager is currently running.
    */
   protected boolean mRunning;
   
   /**
    * The time (in milliseconds) at which this RateAverager started.
    */
   protected long mStartTime;
   
   /**
    * The time (in milliseconds) at which this RateAverager stopped.
    */
   protected long mStopTime;
   
   /**
    * The window length in milliseconds.
    */
   protected long mWindowLength;

   /**
    * Used to store the item count in the current window. This is not a
    * whole number because a window may start or end in the middle of
    * an item.
    */
   protected double mWindowItemCount;
   
   /**
    * The last time at which items were added to a window.
    */
   protected long mLastItemAddTime;
   
   /**
    * The total item count since the RateAverage started.
    */
   protected long mTotalItemCount;
   
   /**
    * Creates a new RateAverager with a default window length of
    * 10 milliseconds.
    */
   public RateAverager()   
   {
      this(10);
   }
   
   /**
    * Creates a new RateAverager.
    * 
    * @param windowLength the length of the window to use in milliseconds.
    */
   public RateAverager(long windowLength)
   {
      // set the window length
      setWindowLength(windowLength);
      
      // reset
      reset();
   }
   
   /**
    * Resets the calculated data and times for this RateAverager.
    */
   protected void reset()
   {
      // reset start and stop times
      setStartTime(0);
      setStopTime(0);
      
      // reset window data
      mWindowItemCount = 0.0D;
      mLastItemAddTime = System.currentTimeMillis();
      
      // reset total data
      mTotalItemCount = 0;
   }   
   
   /**
    * Sets the time (in milliseconds) at which this RateAverager started.
    * 
    * @param time the time (in milliseconds) at which this RateAverager started.
    */
   protected void setStartTime(long time)
   {
      mStartTime = time;
   }
   
   /**
    * Sets the time (in milliseconds) at which this RateAverager stopped.
    * 
    * @param time the time (in milliseconds) at which this RateAverager stopped.
    */
   protected void setStopTime(long time)
   {
      mStopTime = time;
   }

   /**
    * Gets the number of items in the current window.
    *
    * @param now the current time.
    *
    * @return the number of items in the current window.
    */
   protected double getWindowItemCount(double now)
   {
      double rval = 0.0D;
      
      
      
      // FIXME:
      
      return rval;
   }
   
   /**
    * Gets the rate in items per millisecond given items and a time interval
    * in milliseconds.
    * 
    * @param items the items in the interval.
    * @param interval the time interval in milliseconds.
    * 
    * @return the rate in items per second.
    */
   protected double getItemsPerMillisecond(double items, double interval)   
   {
      // items / millisecond -- force interval to 1
      return items / Math.max(1, interval);
   }
   
   /**
    * Gets the rate in items per second given items and a time interval
    * in milliseconds.
    * 
    * @param items the items in the interval.
    * @param interval the time interval in milliseconds.
    * 
    * @return the rate in items per second.
    */
   protected double getItemsPerSecond(double items, double interval)
   {
      // items / millisecond over 1000 = items / second
      return getItemsPerMillisecond(items, interval) / 1000.0D;      
   }
   
   /**
    * Starts this RateAverager if it hasn't already started. When a
    * RateAverager starts, it clears all of its previously calculated data.
    */
   public synchronized void start()
   {
      if(!isRunning())
      {
         // reset
         reset();
         
         // set start time
         setStartTime(System.currentTimeMillis());
         
         getLogger().detail(getClass(), "RateAverager started.");
      }
      else
      {
         getLogger().detail(getClass(), "RateAverager already started.");
      }
   }

   /**
    * Stops this RateAverager if it is running. The calculated data for this
    * RateAverager will not be cleared when the RateAverager stops, it will
    * be cleared if start() is called again afterwards.
    */
   public synchronized void stop()
   {
      if(isRunning())
      {
         // set stop time
         setStopTime(System.currentTimeMillis());
         
         getLogger().detail(getClass(), "RateAverager stopped.");
      }
      else
      {
         getLogger().detail(getClass(), "RateAverager already stopped.");
      }
   }
   
   /**
    * Gets whether or not this RateAverager is running.
    * 
    * @return true if this RateAverager is running, false if not.
    */
   public synchronized boolean isRunning()
   {
      return mRunning;
   }
   
   /**
    * Adds items to the current window.
    * 
    * @param items the items to add to the current window.
    * @param interval the interval (in milliseconds) underwhich the items
    *                 were transferred.
    */
   public void addWindowItems(long items, long interval)
   {
      // determine the rate for the items
      double rate = getItemsPerMillisecond(items, interval);
      
      // get the time passed since the last item add time
      // FIXME:
      
      // add items as if the rate had been spread across the whole window
      mWindowItemCount += rate * getWindowLength();
      
      // update the last item add time
      mLastItemAddTime = System.currentTimeMillis();
   }
   
   /**
    * Sets the window length in milliseconds. The larger the window, the
    * more accurate an average rate will be over a long period of time,
    * the smaller the window the more accurate an average rate will be
    * over short periods of time.
    * 
    * @param length the length of the window in milliseconds >= 1.
    */
   public void setWindowLength(long length)
   {
      mWindowLength = Math.max(1, length);
   }
   
   /**
    * Sets the window length in milliseconds. The larger the window, the
    * more accurate an average rate will be over a long period of time,
    * the smaller the window the more accurate an average rate will be
    * over short periods of time.
    * 
    * @return the length of the window in milliseconds >= 1.
    */
   public long getWindowLength()
   {
      return mWindowLength;
   }
   
   /**
    * Gets the time (in milliseconds) at which this RateAverager started. If
    * this RateAverager has not started, 0 is returned.
    * 
    * @return the time (in milliseconds) at which this RateAverager started.
    */
   public long getStartTime()
   {
      return mStartTime;
   }
   
   /**
    * Gets the time (in milliseconds) at which this RateAverager stopped. If
    * this RateAverager has not stopped, 0 is returned.
    * 
    * @return the time (in milliseconds) at which this RateAverager stopped.
    */
   public long getStopTime()
   {
      return mStopTime;
   }
   
   /**
    * Gets the total amount of time that this RateAverager has been
    * running or was run.
    * 
    * If this RateAverager is currently running, the amount of time it
    * has been running is returned.
    * 
    * If this RateAverager was started and then stopped, then total amount
    * of time it ran is returned.
    * 
    * If this RateAverager has not started, 0 is returned.
    * 
    * @return the amount of time (in milliseconds) that this RateAverager
    *         has been running or was run.
    */
   public long getRunTime()
   {
      long rval = 0;
      
      if(isRunning())
      {
         rval = System.currentTimeMillis() - getStartTime();
      }
      else
      {
         rval = getStopTime() - getStartTime();
      }
      
      return rval;
   }

   /**
    * Gets the current rate in items per second. The current rate is the
    * average rate in the current time window.
    * 
    * If the RateAverager has not started, then 0 is returned.
    * 
    * @return the current rate in items per second.
    */
   public double getCurrentRate()
   {
      double rval = 0.0D;
      
      // make sure that the start time is not 0
      if(getStartTime() != 0)
      {
         // get the time
         double now = getRunTime();
         
         // get the items
         double items = getWindowItemCount(now);
         
         // get the interval
         double interval = now - getWindowLength();
         
         // get the rate in items per second
         rval = getItemsPerSecond(items, interval);
      }
      
      return rval;      
   }
   
   /**
    * Gets the total rate since the RateAverager was started in items
    * per second.
    * 
    * The total rate is the total number of items that have passed through
    * this RateAverager over the amount of time this RateAverager has
    * been running or was run.
    * 
    * If the RateAverager has not started, then 0 is returned.
    * 
    * @return the total rate since the RateAverager was started.
    */
   public double getTotalRate()
   {
      double rval = 0.0D;
      
      // make sure that the start time is not 0
      if(getStartTime() != 0)
      {
         // get the rate in items per second
         rval = getItemsPerSecond(mTotalItemCount, getRunTime());
      }
      
      return rval;
   }
   
   /**
    * Gets the logger for this RateAverager.
    * 
    * @return the logger for this RateAverager.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }   
}
