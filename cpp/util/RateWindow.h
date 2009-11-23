/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_util_TimeWindow_H
#define db_util_TimeWindow_H

#include <inttypes.h>

namespace db
{
namespace util
{

/**
 * A RateWindow keeps track of an increasing number of items over some
 * period of time. It can provide the rate at which the number of items
 * increased over that period of time and whether or not a given time
 * falls within the window.
 *
 * A RateWindow's time is measured in milliseconds. If the window length
 * is greater than 0, the RateWindow has a start time and an end time. These
 * can be set to an absolute system time or according to whatever other
 * system the user employs.
 *
 * There are two different time measurements within a RateWindow:
 *
 * 1. The passed time. This refers to the amount of time since start time.
 * 2. The item time. This refers to how much time has been spent accumulating
 *    items. It is used for rate calculations and does not need to match
 *    the passed time.
 *
 * The most common use of this class is to initialize a RateWindow and then
 * to make repeated calls to addItems() while the system time is within the
 * RateWindow's boundaries. Using this method will result in this class
 * automatically calculating a reasonable accurate rate of item accumulation.
 * Using the other methods may require more manual intervention to ensure rates
 * are accurate.
 *
 * This class does not provide any internal locking or thread synchronization,
 * this must be performed appropriately by the user of this class.
 *
 * @author Dave Longley
 */
class RateWindow
{
protected:
   /**
    * The length of this window in milliseconds.
    */
   uint64_t mLength;

   /**
    * The time at which this window started in milliseconds.
    */
   uint64_t mStartTime;

   /**
    * The time that has passed within this window.
    */
   uint64_t mTimePassed;

   /**
    * The number of items in this window.
    */
   uint64_t mItemCount;

   /**
    * The amount of time (in milliseconds) that has passed whilst accumulating
    * items.
    */
   uint64_t mItemTime;

   /**
    * The earliest time that addItems() was called.
    */
   uint64_t mEarliestAddTime;

   /**
    * The last time that addItems() was called.
    */
   uint64_t mLastAddTime;

public:
   /**
    * Creates a new empty RateWindow with the given maximum length for
    * this window. If a maximum length of 0 is specified then this RateWindow
    * will have no maximum length.
    *
    * @param length the maximum length of this window (in milliseconds).
    */
   RateWindow(uint64_t length = 0);

   /**
    * Destructs this RateWindow.
    */
   virtual ~RateWindow();

   /**
    * Resets this window. This method will set the start time, passed time,
    * item count, and item time in this window to 0.
    */
   virtual void reset();

   /**
    * Sets this window equal to another window. The length, start time,
    * passed time, item count, and item time of the other window will be
    * copied to this window.
    *
    * @param window the window to set this window equal to.
    */
   virtual void setEqualTo(RateWindow& window);

   /**
    * Sets this window length in milliseconds. A value of 0 indicates
    * no maximum length. The current item count and item time will be
    * unaffected.
    *
    * @param length the length of this window in milliseconds.
    */
   virtual void setLength(uint64_t length);

   /**
    * Gets this window length in milliseconds. A value of 0 indicates
    * no maximum length.
    *
    * @return the length of this window in milliseconds.
    */
   virtual uint64_t getLength();

   /**
    * Sets the time at which this window started in milliseconds. The
    * current item count will be unaffected.
    *
    * @param time the time at which this window started in milliseconds.
    */
   virtual void setStartTime(uint64_t time);

   /**
    * Gets the time at which this window started in milliseconds.
    *
    * @return the time at which this window started in milliseconds.
    */
   virtual uint64_t getStartTime();

   /**
    * Gets the time (in milliseconds) at which this window ends. This
    * returns 0 if there is no maximum length for this window. The end
    * time is not considered "within" the window.
    *
    * @return the time at which this window ends (in milliseconds).
    */
   virtual uint64_t getEndTime();

   /**
    * Sets the current time (in milliseconds) in this window. This will
    * affect the passed time. This will not affect the item time.
    *
    * Do not set this before the start time of the window.
    *
    * @param time the current time (in milliseconds) in this window.
    */
   virtual void setCurrentTime(uint64_t time);

   /**
    * Gets the current time (in milliseconds) in this window. The current
    * time in this window is the start time plus the passed time in this
    * window.
    *
    * @return the current time (in milliseconds) in this window.
    */
   virtual uint64_t getCurrentTime();

   /**
    * Gets the amount of time (in milliseconds) left in this window. The
    * amount of time left in this window is the end time minus the current
    * time.
    *
    * @return the amount of time (in milliseconds) left in this window.
    */
   virtual uint64_t getRemainingTime();

   /**
    * Sets the time (in milliseconds) passed in this window. This will not
    * affect the item time.
    *
    * @param time the amount of time (in milliseconds) passed in this window.
    */
   virtual void setTimePassed(uint64_t time);

   /**
    * Gets the amount of time (in milliseconds) that has passed in this window.
    *
    * @return the amount of time (in milliseconds) that has passed in this
    *         window.
    */
   virtual uint64_t getTimePassed();

   /**
    * Increases the amount of time (in milliseconds) passed since the start
    * time in this window.
    *
    * @param time the interval of time (in milliseconds) to add to this
    *             window.
    */
   virtual void increaseTimePassed(uint64_t time);

   /**
    * Returns true if the passed time (in milliseconds) falls within this
    * window, false if not.
    *
    * @param time the time (in milliseconds) to check against this window.
    *
    * @return true if the passed time is in this window, false if not.
    */
   virtual bool isTimeInWindow(uint64_t time);

   /**
    * Sets the item count in this window. The item time will be unaffected.
    *
    * @param count the new item count.
    */
   virtual void setItemCount(uint64_t count);

   /**
    * Gets the item count for this window.
    *
    * @return the item count for this window.
    */
   virtual uint64_t getItemCount();

   /**
    * Sets the item time in this window.
    *
    * @param time the new item time.
    */
   virtual void setItemTime(uint64_t time);

   /**
    * Gets the item time for this window.
    *
    * @return the item time for this window.
    */
   virtual uint64_t getItemTime();

   /**
    * Adds items to this RateWindow. This will affect the item count and
    * the item time.
    *
    * The item count will be increased by the number of items given.
    *
    * The item time will be increased by any interval of time between
    * start and now that has not already been counted. This accounts for
    * concurrent processes that may be adding items to the same RateWindow
    * by ensuring that overlapping time periods are not double-counted.
    *
    * Note: It is a programmer error to pass in a start time that is beyond
    * now.
    *
    * If a start time is given that is before the start of this window, then
    * only a portion of the given item count will be added to this window
    * according to the linear rate at which items are assumed to have accrued,
    * namely, roundup(count/(now - start)) * (time within window). The item
    * time will also only be increased by time spent within the window.
    *
    * @param count the amount of items to increase by.
    * @param start the time at which items began accumulating.
    * @param now the current time.
    */
   virtual void addItems(uint64_t count, uint64_t start, uint64_t now);

   /**
    * Gets the current rate at which the item count is increasing in
    * this window. The current rate is the item count over the item time.
    *
    * @return the current rate in items per millisecond.
    */
   virtual double getItemsPerMillisecond();

   /**
    * Gets the current rate at which the item count is increasing in
    * this window. The current rate is the item count over the item time.
    *
    * @return the current rate in items per second.
    */
   virtual double getItemsPerSecond();

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
   static double calcItemsPerMillisecond(double items, double interval);

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
   static double calcItemsPerSecond(double items, double interval);
};

} // end namespace util
} // end namespace db
#endif
