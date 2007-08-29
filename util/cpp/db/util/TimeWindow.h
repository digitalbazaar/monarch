/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_TimeWindow_H
#define db_util_TimeWindow_H

#include "db/rt/Object.h"

namespace db
{
namespace util
{

/**
 * A time window keeps track of an increasing number of items over some
 * period of time. It can provide the rate at which the number of items
 * increased over that period of time.
 * 
 * The time used in this window can be absolute or relative.
 * 
 * @author Dave Longley
 */
class TimeWindow : public virtual db::rt::Object
{
protected:
   /**
    * The length of this window in milliseconds.
    */
   unsigned long long mLength;
   
   /**
    * The time at which this window started in milliseconds.
    */
   unsigned long long mStartTime;
   
   /**
    * The number of items in this window.
    */
   unsigned long long mItemCount;
   
   /**
    * The amount of time (in milliseconds) that has passed in this window.
    */
   unsigned long long mTimePassed;
   
   /**
    * The last time that time (in milliseconds) that time was added to this
    * window. This is used to help convenience methods that can add the amount
    * of time (in milliseconds) since the last addition of time to this
    * window.
    * 
    * Uses absolute time only.
    */
   unsigned long long mLastAddTime;
   
public:
   /**
    * Creates a new empty TimeWindow with the given maximum length for
    * this window. If a maximum length of 0 is specified then this TimeWindow
    * will have no maximum length.
    * 
    * @param length the maximum length of this time window (in milliseconds).
    */
   TimeWindow(unsigned long long length = 0);
   
   /**
    * Destructs this TimeWindow.
    */
   virtual ~TimeWindow();
   
   /**
    * Adjusts the current item count according to some change in window
    * time. The change in window time can be at the beginning of the window
    * (a change in the start time) or at the end of the window (a change
    * in window length).
    * 
    * @param timeChange the change in window time.
    */
   virtual void adjustItemCount(unsigned long long timeChange);
   
   /**
    * Resets this window. This method will restore the start time, item
    * count, and time passed in this window to 0.
    * 
    * This method will also reset the last time that time was added to
    * this interval to the current time.
    */
   virtual void reset();
   
   /**
    * Sets this window equal to another window. The length, start time,
    * item count, time passed, and the last add time of the other window
    * will be copied to this window.
    * 
    * @param window the window to set this window equal to.
    */
   virtual void setEqualTo(TimeWindow& window);
   
   /**
    * Gets the current rate at which the item count is increasing in
    * this window. The current rate is the item count over the amount of
    * time passed within this window.
    * 
    * @return the current rate in items per second.
    */
   virtual double getIncreaseRateInItemsPerMillisecond();
   
   /**
    * Gets the current rate at which the item count is increasing in
    * this window. The current rate is the item count over the amount of
    * time passed within this window.
    * 
    * @return the current rate in items per second.
    */
   virtual double getIncreaseRate();
   
   /**
    * Sets this window length in milliseconds. A value of 0 indicates
    * no maximum length. The current item count will be unaffected.
    * 
    * @param length the length of this window in milliseconds.
    */
   virtual void setLength(unsigned long long length);
   
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
   virtual void setLength(unsigned long long length, bool adjust);
   
   /**
    * Gets this window length in milliseconds. A value of 0 indicates
    * no maximum length.
    * 
    * @return the length of this window in milliseconds.
    */
   virtual unsigned long long getLength();
   
   /**
    * Sets the time at which this window started in milliseconds. The
    * current item count will be unaffected.
    * 
    * @param time the time at which this window started in milliseconds.
    */
   virtual void setStartTime(unsigned long long time);
   
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
   virtual void setStartTime(unsigned long long time, bool adjust);
   
   /**
    * Gets the time at which this window started in milliseconds.
    * 
    * @return the time at which this window started in milliseconds.
    */
   virtual unsigned long long getStartTime();
   
   /**
    * Gets the time (in milliseconds) at which this window ends. This
    * returns 0 if there is no maximum length for this window.
    * 
    * @return the time at which this window ends (in milliseconds).
    */
   virtual unsigned long long getEndTime();
   
   /**
    * Gets the current time (in milliseconds) in this window. The current
    * time in this window is the start time plus the time passed in this window.
    * 
    * @return the current time (in milliseconds) in this window.
    */
   virtual unsigned long long getCurrentTime();
   
   /**
    * Gets the amount of time (in milliseconds) left in this window. The
    * amount of time left in this window is the end time minus the current
    * time. 
    * 
    * @return the amount of time (in milliseconds) left in this window.
    */
   virtual unsigned long long getRemainingTime();
   
   /**
    * Returns true if the passed time  (in milliseconds) falls within this
    * window, false if not.
    * 
    * @param time the time (in milliseconds) to check against this window.
    * 
    * @return true if the passed time is in this window, false if not.
    */
   virtual bool isTimeInWindow(unsigned long long time);
   
   /**
    * Increases the item count in this window. The amount of time passed
    * in this window is unaffected by this method.
    * 
    * There is no measure in place to ensure that items aren't added to
    * this window once it has already ended.
    * 
    * @param increase the amount of items to increase by.
    */
   virtual void increaseItemCount(unsigned long long increase);
   
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
   virtual void increaseItemCount(
      unsigned long long increase, unsigned long long interval);
   
   /**
    * Adds time (in milliseconds) to this window.
    *  
    * @param time the interval of time (in milliseconds) to add to
    *             this window.
    */
   virtual void increaseTimePassed(unsigned long long time);
   
   /**
    * Adds the amount of time (in milliseconds) since the last time
    * that time was added to this window to this window. If time was never
    * added to this window, then the time since the last time this
    * window was reset will be added to this window.
    */
   virtual void increaseTimePassedWithCurrentTime();
   
   /**
    * Gets the item count for this window.
    * 
    * @return the item count for this window.
    */
   virtual unsigned long long getItemCount();
   
   /**
    * Gets the amount of time (in milliseconds) that has passed in this window.
    * 
    * @return the amount of time (in milliseconds) that has passed in this
    *         window.
    */
   virtual unsigned long long getTimePassed();
   
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
   static double getItemsPerMillisecond(double items, double interval);
   
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
   static double getItemsPerSecond(double items, double interval);
};

} // end namespace util
} // end namespace db
#endif
