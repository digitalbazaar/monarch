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
 * The time used in this window can be absolute or relative.
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
    * The amount of time (in milliseconds) that has passed in this window.
    */
   protected long mTimePassed;
   
   /**
    * The last time that time (in milliseconds) that time was added to this
    * window. This is used to help convenience methods that can add the amount
    * of time (in milliseconds) since the last addition of time to this
    * window.
    * 
    * Uses absolute time only.
    */
   protected long mLastAddTime;
   
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
      // multiply the current rate by the time change and
      // increase the item count accordingly
      increaseItemCount(Math.round(getIncreaseRate() * timeChange));      
   }
   
   /**
    * Resets this window. This method will restore the start time, item
    * count, and time passed in this window to 0.
    * 
    * This method will also reset the last time that time was added to
    * this interval to the current time.
    */
   public synchronized void reset()
   {
      mStartTime = 0;
      mItemCount = 0;
      mTimePassed = 0;
      
      // reset the last time that time was added
      mLastAddTime = System.currentTimeMillis();
   }
   
   /**
    * Sets this window equal to another window. The length, start time,
    * item count, time passed, and the last add time of the other window
    * will be copied to this window.
    * 
    * @param window the window to set this window equal to.
    */
   public synchronized void setEqualTo(TimeWindow window)
   {
      mLength = window.getLength();
      mStartTime = window.getStartTime();
      mItemCount = window.getItemCount();
      mTimePassed = window.getTimePassed();
      mLastAddTime = window.mLastAddTime;
   }
   
   /**
    * Gets the current rate at which the item count is increasing in
    * this window. The current rate is the item count over the amount of
    * time passed within this window.
    * 
    * @return the current rate in items per second.
    */
   public synchronized double getIncreaseRateInItemsPerMillisecond()   
   {
      // get the rate in items per millisecond
      return getItemsPerMillisecond(getItemCount(), getTimePassed());      
   }
   
   /**
    * Gets the current rate at which the item count is increasing in
    * this window. The current rate is the item count over the amount of
    * time passed within this window.
    * 
    * @return the current rate in items per second.
    */
   public synchronized double getIncreaseRate()
   {
      // get the rate in items per second
      return getItemsPerSecond(getItemCount(), getTimePassed());
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
    * Gets the time (in milliseconds) at which this window ends. This
    * returns 0 if there is no maximum length for this window.
    * 
    * @return the time at which this window ends (in milliseconds).
    */
   public long getEndTime()
   {
      long rval = 0;
      
      if(getLength() > 0)
      {
         rval = getStartTime() + getLength() - 1;
      }
      
      return rval;
   }
   
   /**
    * Gets the current time (in milliseconds) in this window. The current
    * time in this window is the start time plus the time passed in this window.
    * 
    * @return the current time (in milliseconds) in this window.
    */
   public synchronized long getCurrentTime()
   {
      return getStartTime() + getTimePassed();
   }
   
   /**
    * Gets the amount of time (in milliseconds) left in this window. The
    * amount of time left in this window is the end time minus the current
    * time. 
    * 
    * @return the amount of time (in milliseconds) left in this window.
    */
   public synchronized long getRemainingTime()
   {
      return getEndTime() - getCurrentTime();
   }
   
   /**
    * Returns true if the passed time  (in milliseconds) falls within this
    * window, false if not.
    * 
    * @param time the time (in milliseconds) to check against this window.
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
         if(getLength() == 0 || time < getEndTime())
         {
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Increases the item count in this window. The amount of time passed
    * in this window is unaffected by this method.
    * 
    * There is no measure in place to ensure that items aren't added to
    * this window once it has already ended.
    * 
    * @param increase the amount of items to increase by.
    */
   public synchronized void increaseItemCount(long increase)
   {
      mItemCount = Math.max(0, mItemCount + increase);
   }
   
   /**
    * Increases the item count over a period of time. The amount of time
    * passed in this window is increased by the passed interval.
    * 
    * There is no measure in place to ensure that items aren't added to
    * this window once it has already ended.
    * 
    * @param increase the amount of items to increase by.
    * @param interval the interval during which the item increase took place
    *                 (in milliseconds).
    */
   public synchronized void increaseItemCount(long increase, long interval)   
   {
      increaseItemCount(increase);
      increaseTimePassed(interval);
   }
   
   /**
    * Adds time (in milliseconds) to this window.
    *  
    * @param time the interval of time (in milliseconds) to add to
    *             this window.
    */
   public synchronized void increaseTimePassed(long time) 
   {
      mTimePassed = Math.max(0, mTimePassed + time);

      // see if this window has a maximum length
      if(getLength() != 0)
      {
         // cap time passed at the length of the window
         mTimePassed = Math.min(mTimePassed, getLength());
      }
      
      // update last time that time was added
      mLastAddTime = System.currentTimeMillis();
   }
   
   /**
    * Adds the amount of time (in milliseconds) since the last time
    * that time was added to this window to this window. If time was never
    * added to this window, then the time since the last time this
    * window was reset will be added to this window.
    */
   public synchronized void increaseTimePassedWithCurrentTime()
   {
      // add the time since the last add time
      increaseTimePassed(System.currentTimeMillis() - mLastAddTime);
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
    * Gets the amount of time (in milliseconds) that has passed in this window.
    * 
    * @return the amount of time (in milliseconds) that has passed in this
    *         window.
    */
   public long getTimePassed()
   {
      return mTimePassed;
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
