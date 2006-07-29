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
 * during a window of time divided by the time passed in the window produces
 * the average rate.
 * 
 * This RateAverager uses 2 consecutive time windows for storing item count
 * increases. There is a "current window" and a "next window". The next
 * window overlaps the current window by half of its length. When a call is
 * made to the RateAverager to increase the item count, the item counts 
 * of the two windows are updated according to whether or not the current
 * time falls within their ranges.
 * 
 * The current window will move to the next window and a new next window
 * will be created as time progresses.
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
    * The current time window.
    */
   protected TimeWindow mCurrentWindow;
   
   /**
    * The next time window.
    */
   protected TimeWindow mNextWindow;
   
   /**
    * The total item count since the RateAverage started.
    */
   protected long mTotalItemCount;
   
   /**
    * Creates a new RateAverager a default window length of 1 second.
    */
   public RateAverager()
   {
      this(1000);
   }
   
   /**
    * Creates a new RateAverager with the given window length.
    * 
    * The longer the window length, the more accurate an average rate will
    * be over a long period of time, the shorter the window length the more
    * accurate an average rate will be over a shorter period of time.
    * 
    * @param windowLength the length of the window to use in milliseconds.
    */
   public RateAverager(long windowLength)
   {
      // create the windows
      createWindows(windowLength);
      
      // reset
      reset();
   }
   
   /**
    * Creates the time windows.
    * 
    * @param windowLength the length of the windows to use in milliseconds.
    */
   protected void createWindows(long windowLength)
   {
      // create current window
      mCurrentWindow = new TimeWindow(windowLength);
      
      // create next window
      mNextWindow = new TimeWindow(windowLength);
   }
   
   /**
    * Resets the calculated data and times for this RateAverager.
    */
   protected void reset()
   {
      // reset start and stop times
      setStartTime(0);
      setStopTime(0);
      
      // reset the windows
      resetWindows();
      
      // reset total item count
      mTotalItemCount = 0;
   }
   
   /**
    * Resets the time windows.
    */
   protected void resetWindows()
   {
      getCurrentWindow().reset();
      getNextWindow().reset();
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
    * Updates the current and next windows with a new start time. This will
    * reset the windows.
    * 
    * @param time the new start time for the current window.
    */
   protected void setWindowStartTimes(long time)
   {
      // reset the windows
      resetWindows();
      
      // set current window start time
      getCurrentWindow().setStartTime(time);
      
      // set next window start time
      getNextWindow().setStartTime(time + getHalfWindowLength());
   }
   
   /**
    * Gets half of the current window length. Rounds up.
    * 
    * @return half of the current window length, rounded up.
    */
   protected long getHalfWindowLength()
   {
      return Math.round(getWindowLength() / 2.0D);
   }
   
   /**
    * Updates the current and next windows with a new length.
    * 
    * @param length the new window length.
    */
   protected void updateWindowLengths(long length)
   {
      // set the current window length, adjusting items
      getCurrentWindow().setLength(length, true);
      
      // set the next window length, adjusting items
      getNextWindow().setLength(length, true);
      
      // update the next window start time
      getNextWindow().setStartTime(
         getCurrentWindow().getStartTime() + getHalfWindowLength());
   }
   
   /**
    * Moves the current window to the next window and resets the next
    * window.
    */
   protected void moveCurrentWindow()
   {
      // set the current window to the next window
      getCurrentWindow().setEqualTo(getNextWindow());
      
      // reset the next window
      getNextWindow().reset();
      
      // set the next window start time
      getNextWindow().setStartTime(
         getCurrentWindow().getStartTime() + getHalfWindowLength());
   }
   
   /**
    * Gets the current time window.
    * 
    * @return the current time window.
    */
   protected TimeWindow getCurrentWindow()
   {
      return mCurrentWindow;
   }
   
   /**
    * Gets the next time window.
    * 
    * @return the next time window.
    */
   protected TimeWindow getNextWindow()
   {
      return mNextWindow;
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

         // now running
         mRunning = true;
         
         // get the current time
         long now = System.currentTimeMillis();
         
         // set start time
         setStartTime(now);
         
         // set window start times
         setWindowStartTimes(now);
         
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
         // no longer running
         mRunning = false;
         
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
    * Restarts this rate averager. Calls stop() and then start().
    */
   public synchronized void restart()
   {
      stop();
      start();
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
    * Increases the item count of the current window in this RateAverager.
    * 
    * @param increase the amount to increase the number of items by.
    */
   public synchronized void increaseItemCount(long increase)
   {
      // get the current time
      long now = System.currentTimeMillis();
      
      // see if the current time falls within the current window
      if(getCurrentWindow().isTimeInWindow(now))
      {
         // increase item count of the current window
         getCurrentWindow().increaseItemCount(increase);
         
         // see if the time falls in next window as well
         if(getNextWindow().isTimeInWindow(now))
         {
            // increase item count of the next window
            getNextWindow().increaseItemCount(increase);
         }
      }
      else if(getNextWindow().isTimeInWindow(now))
      {
         // time falls within the next window, but not the current one
         // so update the next window and then move the current window
         getNextWindow().increaseItemCount(increase);
         
         // move current window
         moveCurrentWindow();
      }
      else
      {
         // the windows have already passed, so update window starting times
         setWindowStartTimes(now);
      }
      
      // add items to the total item count
      mTotalItemCount = Math.max(0, mTotalItemCount + increase);
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
   public synchronized long getRunTime()
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
   public synchronized double getCurrentRate()
   {
      double rval = 0.0D;
      
      // make sure that the start time is not 0
      if(getStartTime() != 0)
      {
         // get the current time
         long now = System.currentTimeMillis();
         
         // see if the current time does not fall within the current window
         if(!getCurrentWindow().isTimeInWindow(now))
         {
            // see if the current time falls in the next window
            if(getNextWindow().isTimeInWindow(now))
            {
               // move the current window
               moveCurrentWindow();
            }
            else
            {
               // time is ahead of both windows, so set new start times
               setWindowStartTimes(now);
            }
         }
         
         // get the current window rate
         rval = getCurrentWindow().getIncreaseRate(now);
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
   public synchronized double getTotalRate()
   {
      double rval = 0.0D;
      
      // make sure that the start time is not 0
      if(getStartTime() != 0)
      {
         // get the rate in items per second
         rval = TimeWindow.getItemsPerSecond(mTotalItemCount, getRunTime());
      }
      
      return rval;
   }
   
   /**
    * Sets the window length in milliseconds. The larger the window, the
    * more accurate an average rate will be over a long period of time,
    * the smaller the window the more accurate an average rate will be
    * over short periods of time.
    * 
    * @param length the length of the window in milliseconds >= 2.
    */
   public synchronized void setWindowLength(long length)
   {
      // the window length must be at least two because two windows that
      // are 1/2 of the window length apart are always stored -- and
      // this RateAverager is only accurate to 1 whole millisecond
      length = Math.max(2, length);
      
      // update window lengths
      updateWindowLengths(length);
   }
   
   /**
    * Gets the window length in milliseconds. The larger the window, the
    * more accurate an average rate will be over a long period of time,
    * the smaller the window the more accurate an average rate will be
    * over short periods of time.
    * 
    * @return the length of the window in milliseconds >= 2.
    */
   public long getWindowLength()
   {
      return getCurrentWindow().getLength();
   }
   
   /**
    * Gets the logger for this TimeWindow.
    * 
    * @return the logger for this TimeWindow.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }
}
