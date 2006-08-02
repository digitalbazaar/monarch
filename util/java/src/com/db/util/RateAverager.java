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
 * The time that this RateAverager uses can be relative or absolute.
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
    * The time (in milliseconds) that has passed since this RateAverager
    * started.
    */
   protected long mTimePassed;
   
   /**
    * The last time that time (in milliseconds) that a rate was added to this
    * window. This is used to help convenience methods that can add the amount
    * of time (in milliseconds) since the last addition of time to this
    * window.
    * 
    * Uses absolute time only.
    */
   protected long mLastAddTime;
   
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
      
      // reset time passed
      mTimePassed = 0;
      
      // reset last time a rate was added
      mLastAddTime = System.currentTimeMillis();

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
    * The start time that this RateAverager uses can be relative or absolute.
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
    * The stop time that this RateAverager uses can be relative or absolute.
    * 
    * @param time the time (in milliseconds) at which this RateAverager stopped.
    */
   protected void setStopTime(long time)
   {
      mStopTime = time;
   }
   
   /**
    * Updates the current and next windows with a new start time.
    * 
    * @param time the new start time for the current window.
    */
   protected void setWindowStartTimes(long time)
   {
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
    * 
    * Uses a start time of 0.
    */
   public synchronized void start()   
   {
      start(0);
   }
   
   /**
    * Starts this RateAverager if it hasn't already started. When a
    * RateAverager starts, it clears all of its previously calculated data.
    * 
    * @param time the start time to use (in milliseconds) -- can be absolute
    *             or relative.
    */
   public synchronized void start(long time)
   {
      if(!isRunning())
      {
         // reset
         reset();

         // now running
         mRunning = true;
         
         // set start time
         setStartTime(time);
         
         // set window start times
         setWindowStartTimes(time);
         
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
    * 
    * Uses a stop time of getCurrentTime().
    */
   public synchronized void stop()   
   {
      // stop at the current time
      stop(getCurrentTime());
   }

   /**
    * Stops this RateAverager if it is running. The calculated data for this
    * RateAverager will not be cleared when the RateAverager stops, it will
    * be cleared if start() is called again afterwards.
    * 
    * @param time the stop time to use (in milliseconds) -- can be absolute
    *             or relative.
    */
   public synchronized void stop(long time)
   {
      if(isRunning())
      {
         // no longer running
         mRunning = false;
         
         // set stop time
         setStopTime(time);
         
         getLogger().detail(getClass(), "RateAverager stopped.");
      }
      else
      {
         getLogger().detail(getClass(), "RateAverager already stopped.");
      }
   }
   
   /**
    * Restarts this rate averager. Calls stop() and then start().
    * 
    * Uses a stop time of getCurrentTime() and a start time of 0.
    */
   public synchronized void restart()
   {
      stop(getCurrentTime());
      start(0);
   }   
   
   /**
    * Restarts this rate averager. Calls stop() and then start().
    * 
    * @param time the restart time to use (in milliseconds) -- can be absolute
    *             or relative.
    */
   public synchronized void restart(long time)
   {
      stop(time);
      start(time);
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
    * Adds a new rate to this RateAverager. Increases the item count and
    * the amount of time passed for the current window in this RateAverager.
    * 
    * The amount of time passed is updated by the amount of time that
    * has passed since the last time a rate was added. If a rate hasn't
    * been added yet, then the amount of time since this RateAverager
    * started will be added. 
    * 
    * @param count the amount of items to increase the item count by.
    */
   public synchronized void addRate(long count)   
   {
      // add count and the amount of time passed since the last add
      addRate(count, System.currentTimeMillis() - mLastAddTime);
   }
   
   /**
    * Adds a new rate to this RateAverager. Increases the item count and
    * the amount of time passed for the current window in this RateAverager.
    * 
    * @param count the amount of items to increase the item count by.
    * @param interval the interval overwhich the item count increased
    *                 (in milliseconds).
    */
   public synchronized void addRate(long count, long interval)
   {
      // increase the time passed
      mTimePassed = Math.max(0, mTimePassed + interval);
      
      // get the remaining time in the current window
      long remaining = getCurrentWindow().getRemainingTime();
      
      // see if the interval can be added to the current window without
      // overflowing
      if(interval < remaining)
      {
         // get the overlap time between the current window and the next window
         long overlap = getCurrentWindow().getCurrentTime() + interval -
            getNextWindow().getCurrentTime();
         
         if(overlap > 0)
         {
            // get the portion of the item count in the overlap
            double rate = TimeWindow.getItemsPerMillisecond(count, interval);
            long portion = Math.round(rate * overlap);
            
            // increase the next window count and time
            getNextWindow().increaseItemCount(portion, overlap);
         }
         
         // add the count and interval to the current window
         getCurrentWindow().increaseItemCount(count, interval);
      }
      else
      {
         // there is overflow, so we'll be moving windows
         
         // get the overflow of the interval that cannot be added to
         // the current window
         long overflow = interval - remaining;
         
         // add the amount of time it takes to get the next window up to
         // the end of the current window
         overflow += getCurrentWindow().getEndTime() -
            getNextWindow().getCurrentTime();
         
         // get the remaining time in the next window
         remaining = getNextWindow().getRemainingTime();
         
         // see if the overflow can be added to the next window
         if(overflow < remaining)
         {
            // get the portion of the item count in the overflow
            double rate = TimeWindow.getItemsPerMillisecond(count, interval);
            long portion = Math.round(rate * overflow);
            
            // increase the next window count and time
            getNextWindow().increaseItemCount(portion, overflow);
            
            // move the current window
            moveCurrentWindow();
         }
         else
         {
            // reset the windows
            resetWindows();
            
            // set the current window start time to half of a window - 1
            // before the the current time
            long startTime = getCurrentTime() - getHalfWindowLength() - 1;
            setWindowStartTimes(startTime);

            // get the remainder of the interval that will be used
            long remainder = getCurrentTime() -
               getCurrentWindow().getStartTime();
            
            // add the portion of the item count to the current window
            double rate = TimeWindow.getItemsPerMillisecond(count, interval);
            long portion = Math.round(rate * remainder);
            getCurrentWindow().increaseItemCount(portion, remainder);
         }
      }
      
      // add items to the total item count
      mTotalItemCount = Math.max(0, mTotalItemCount + count);
      
      // update the last rate add time
      mLastAddTime = System.currentTimeMillis();
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
    * Gets the current time in this RateAverager. The current time in this
    * RateAverager is the start time plus the time passed.
    * 
    * @return the current time in this RateAverager.
    */
   public synchronized long getCurrentTime()
   {
      return getStartTime() + getTimePassed();
   }
   
   /**
    * Gets the amount of time (in milliseconds) that has passed since
    * this RateAverager started.
    * 
    * If this RateAverager is currently running, the amount of time it
    * has been running is returned.
    * 
    * If this RateAverager was started and then stopped, then total amount
    * of time it ran is returned.
    * 
    * If this RateAverager has not started, 0 is returned.
    * 
    * @return the amount of time (in milliseconds) that has passed since this
    *         RateAverager started.
    */
   public synchronized long getTimePassed()
   {
      long rval = 0;
      
      if(isRunning())
      {
         rval = mTimePassed;
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
      
      // make sure that the current time is greater than 0
      if(getCurrentTime() > 0)
      {
         // get the current window rate
         rval = getCurrentWindow().getIncreaseRate();
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
      
      // make sure that the current time is greater than 0
      if(getCurrentTime() > 0)
      {
         // get the rate in items per second
         rval = TimeWindow.getItemsPerSecond(mTotalItemCount, getTimePassed());
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
    * Gets an ETA (in seconds) for the given number of items.
    * 
    * @param count the item count to get an ETA for.
    * 
    * @return the amount of time (in seconds) until the passed number of
    *         items is reached according to the current rate. 
    */
   public synchronized long getETA(long count)
   {
      long rval = 0;
      
      if(count > 0)
      {
         // multiply the current rate by the count
         rval = Math.round(count / getCurrentRate());
      }
      
      return rval;
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
