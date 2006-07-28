/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A time window keeps track of an increasing number of items over some
 * period of time. It can provide the rate at which the number of items
 * increased over that period of time.
 * 
 * @author Dave Longley
 */
public class TimeWindow
{
   /**
    * The length of this window in milliseconds.
    */
   protected long mLength;
   
   /**
    * The time at which this window started in milliseconds.
    */
   protected long mStartTime;

   /**
    * The number of items in this window.
    */
   protected long mItemCount;
   
   /**
    * Creates a new empty TimeWindow with no maximum length.
    */
   public TimeWindow()
   {
      this(0);
   }

   /**
    * Creates a new empty TimeWindow with the given maximum length for
    * this window. If a maximum length of 0 is specified then this TimeWindow
    * will have no maximum length.
    * 
    * @param length the maximum length of this time window (in milliseconds).
    */
   public TimeWindow(long length)
   {
      // set the window length
      setLength(length);
      
      // reset
      reset();
   }
   
   /**
    * Adjusts the current item count according to some change in window
    * time. The change in window time can be at the beginning of the window
    * (a change in the start time) or at the end of the window (a change
    * in window length).
    * 
    * @param timeChange the change in window time.
    */
   protected synchronized void adjustItemCount(long timeChange)
   {
      // get the current time
      long now = System.currentTimeMillis();
      
      // multiply the current rate by the time change and
      // increase the item count accordingly
      increaseItemCount(Math.round(getIncreaseRate(now) * timeChange));      
   }
   
   /**
    * Resets this window. This method will restore the start time and
    * item count for this window to 0.
    */
   public synchronized void reset()
   {
      mStartTime = 0;
      mItemCount = 0;
   }
   
   /**
    * Sets this window equal to another window. The length, start time,
    * and item count of the other window will be copied to this window.
    * 
    * @param window the window to set this window equal to.
    */
   public synchronized void setEqualTo(TimeWindow window)
   {
      mLength = window.getLength();
      mStartTime = window.getStartTime();
      mItemCount = window.getItemCount();
   }
   
   /**
    * Gets the current rate at which the item count is increasing in
    * this window. The current rate is the item count over the amount of
    * time passed within this window.
    * 
    * @param now the current time.
    * 
    * @return the current rate in items per second.
    */
   public synchronized double getIncreaseRateInItemsPerMillisecond(long now)   
   {
      // get the rate in items per millisecond
      return getItemsPerMillisecond(getItemCount(), getInterval(now));      
   }
   
   /**
    * Gets the current rate at which the item count is increasing in
    * this window. The current rate is the item count over the amount of
    * time passed within this window.
    * 
    * @param now the current time.
    * 
    * @return the current rate in items per second.
    */
   public synchronized double getIncreaseRate(long now)
   {
      // get the rate in items per second
      return getItemsPerSecond(getItemCount(), getInterval(now));
   }
   
   /**
    * Sets this window length in milliseconds. A value of 0 indicates
    * no maximum length. The current item count will be unaffected.
    * 
    * @param length the length of this window in milliseconds.
    */
   public void setLength(long length)   
   {
      setLength(length, false);
   }
   
   /**
    * Sets this window length in milliseconds. A value of 0 indicates
    * no maximum length.
    * 
    * The second parameter specifies whether or not the item count should
    * be adjusted proportionately (according to the current rate) by the
    * amount of change to the length.
    * 
    * @param length the length of this window in milliseconds.
    * @param adjust true to adjust the item count proportionately, false
    *               to leave the item count alone.
    */
   public synchronized void setLength(long length, boolean adjust)
   {
      length = Math.max(0, length);

      // if adjusting the item count
      if(adjust)
      {
         // get the time change
         long timeChange = length - mLength;
         
         // adjust the item count
         adjustItemCount(timeChange);
      }
      
      // set new length
      mLength = length;
   }
   
   /**
    * Gets this window length in milliseconds. A value of 0 indicates
    * no maximum length.
    * 
    * @return the length of this window in milliseconds.
    */
   public long getLength()
   {
      return mLength;
   }
   
   /**
    * Sets the time at which this window started in milliseconds. The
    * current item count will be unaffected.
    * 
    * @param time the time at which this window started in milliseconds.
    */
   public void setStartTime(long time)
   {
      setStartTime(time, false);
   }
   
   /**
    * Sets the time at which this window started in milliseconds.
    * 
    * The second parameter specifies whether or not the item count should
    * be adjusted proportionately (according to the current rate) by the
    * amount of change to the length.
    * 
    * @param time the time at which this window started in milliseconds.
    * @param adjust true to adjust the item count proportionately, false
    *               to leave the item count alone.
    */
   public synchronized void setStartTime(long time, boolean adjust)
   {
      time = Math.max(0, time);
      
      // if adjusting the item count
      if(adjust)
      {
         // get the time change
         long timeChange = time - mStartTime;
         
         // adjust the item count
         adjustItemCount(timeChange);
      }
      
      // set new start time
      mStartTime = time;
   }
   
   /**
    * Gets the time at which this window started in milliseconds.
    * 
    * @return the time at which this window started in milliseconds.
    */
   public long getStartTime()
   {
      return mStartTime;
   }
   
   /**
    * Gets the time at which this window ends. This returns 0 if there
    * is no maximum length for this window.
    * 
    * @return the time at which this window ends.
    */
   public long getEndTime()
   {
      long rval = 0;
      
      if(getLength() > 0)
      {
         rval = getStartTime() + getLength();
      }
      
      return rval;
   }
   
   /**
    * Returns true if the passed time falls within this window, false
    * if not.
    * 
    * @param time the time to check against this window.
    * 
    * @return true if the passed time is in this window, false if not.
    */
   public synchronized boolean isTimeInWindow(long time)
   {
      boolean rval = false;
      
      // make sure time is passed start time
      if(time >= getStartTime())
      {
         // make that there is no maximum length or the time falls within
         // the maximum length
         if(getLength() == 0 || time <= getEndTime())
         {
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Increases the item count in this window. There is no measure in place
    * to ensure that items aren't added to this window once it has already
    * ended.
    * 
    * @param increase the amount of items to increase by.
    */
   public synchronized void increaseItemCount(long increase)
   {
      mItemCount = Math.max(0, mItemCount + increase);
   }
   
   /**
    * Gets the item count for this window.
    * 
    * @return the item count for this window.
    */
   public long getItemCount()
   {
      return mItemCount;
   }
   
   /**
    * Gets the current interval between now (the time passed) and the
    * start of the window, if the window has not ended. If the window
    * has ended, then the window length is returned.
    * 
    * @param now the current time.
    * 
    * @return the current interval in this window.
    */
   public synchronized long getInterval(long now)
   {
      // if the time is not in this window, then use the end time
      if(!isTimeInWindow(now))
      {
         now = getEndTime();
      }
      
      // get the current interval
      return now - getStartTime();
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

   /**
    * Gets the rate in items per millisecond given items and a time interval
    * in milliseconds. The passed time interval will be rounded up to 1
    * millisecond if it is less than 1.
    * 
    * @param items the items in the interval.
    * @param interval the time interval in milliseconds.
    * 
    * @return the rate in items per second.
    */
   public static double getItemsPerMillisecond(double items, double interval)   
   {
      // items / millisecond -- force interval to 1
      return items / Math.max(1, interval);
   }

   /**
    * Gets the rate in items per second given items and a time interval
    * in milliseconds. The passed time interval will be rounded up to 1
    * millisecond if it is less than 1.
    * 
    * @param items the items in the interval.
    * @param interval the time interval in milliseconds.
    * 
    * @return the rate in items per second.
    */
   public static double getItemsPerSecond(double items, double interval)
   {
      // items / millisecond * 1000 = items / second
      return getItemsPerMillisecond(items, interval) * 1000.0D;      
   }
}
