/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_util_RateAverager_H
#define db_util_RateAverager_H

#include "db/rt/ExclusiveLock.h"
#include "db/util/TimeWindow.h"

namespace db
{
namespace util
{

/**
 * A RateAverager is a utility class that is used to measure the rate
 * at which items pass through it over a period of time. It keeps track
 * of both a total rate and a current average rate. To produce the current
 * average rate with a reasonable degree of accuracy, it uses a sliding
 * time window.
 *
 * The total number of items that have passed through this RateAverager
 * during a window of time divided by the time passed in the window produces
 * the current average rate.
 *
 * More specifically, this RateAverager uses 2 consecutive time windows for
 * storing item count increases. There is a "current window" and a
 * "next window". The next window overlaps the current window by half of its
 * length. When a call is made to the RateAverager to increase the item count,
 * the item counts of the two windows are updated according to whether or not
 * the current time falls within their ranges.
 *
 * The current window will move to the next window and a new next window
 * will be created as time progresses.
 *
 * To use a RateAverager, it must first be started. To update the number of
 * items that have passed through the RateAverager, call addItems() and
 * provide the number of items and the time at which the items you are
 * adding started accumulating. For instance, if a RateAverager is being
 * used to measure the number of bytes being read from some source, the
 * user of the RateAverager should record the absolute time, in milliseconds,
 * and then call the appropriate read method. Upon returning from the read,
 * the user should pass the number of bytes read (as "items") and the time the
 * read began to the RateAverager via addItems().
 *
 * @author Dave Longley
 */
class RateAverager
{
protected:
   /**
    * The total item count since the RateAverager started.
    */
   uint64_t mItemCount;

   /**
    * The time (in milliseconds) that has passed since this RateAverager
    * started.
    */
   uint64_t mTimePassed;

   /**
    * The earliest time that that was passed to addItems() so far. This is
    * used to help ensure overlapping time segments are not double-counted.
    */
   uint64_t mEarliestAddTime;

   /**
    * The last time that time (in milliseconds) that a rate was added to this
    * window. This is used to help ensure overlapping time segments are not
    * double-counted.
    */
   uint64_t mLastAddTime;

   /**
    * The current time window.
    */
   TimeWindow mCurrentWindow;

   /**
    * The next time window.
    */
   TimeWindow mNextWindow;

   /**
    * A lock for synchronizing this rate averager.
    */
   db::rt::ExclusiveLock mLock;

public:
   /**
    * Creates a new RateAverager with the given window length.
    *
    * The longer the window length, the more accurate an average rate will
    * be over a long period of time, the shorter the window length the more
    * accurate an average rate will be over a shorter period of time.
    *
    * @param windowLength the length of the window to use in milliseconds.
    */
   RateAverager(uint64_t windowLength = 1000);

   /**
    * Destructs this RateAverager.
    */
   virtual ~RateAverager();

   /**
    * Resets the calculated data and times for this RateAverager.
    */
   virtual void reset();

   /**
    * Adds items to this RateAverager. This will affect the total item count
    * and time passed and the item count and amount of time passed in the
    * current window.
    *
    * The total item count will be increased by the number of items given and
    * the total time passed will be increased by any interval of time between
    * start and now that has not already been counted. This accounts for
    * concurrent processes that may be adding items to the same RateAverager
    * by ensuring that overlapping time periods are not double-counted.
    *
    * Note: It is a programmer error to pass in a start time that is beyond
    * now.
    *
    * @param count the amount of items to increase the item count by.
    * @param start the time at which the items began accumulating.
    */
   virtual void addItems(uint64_t count, uint64_t start);

   /**
    * Gets the amount of time (in milliseconds) that have passed while adding
    * items to this RateAverager. This time will only change when items are
    * added.
    *
    * @return the amount of time (in milliseconds) that has passed while
    *         adding items to this RateAverager.
    */
   virtual uint64_t getTimePassed();

   /**
    * Gets the current rate in items per millisecond. The current rate is the
    * average rate in the current time window.
    *
    * If no time has yet passed, then 0 is returned.
    *
    * @return the current rate in items per millisecond.
    */
   virtual double getItemsPerMillisecond();

   /**
    * Gets the current rate in items per second. The current rate is the
    * average rate in the current time window.
    *
    * If no time has yet passed, then 0 is returned.
    *
    * @return the current rate in items per second.
    */
   virtual double getItemsPerSecond();

   /**
    * Gets the total rate of all addItems() calls to this RateAverager in items
    * per millisecond.
    *
    * The total rate is the total number of items that have passed through
    * this RateAverager over the amount of time passed in this RateAverager.
    *
    * @return the total rate recorded in items per millisecond.
    */
   virtual double getTotalItemsPerMillisecond();

   /**
    * Gets the total rate of all addItems() calls to this RateAverager in items
    * per second.
    *
    * The total rate is the total number of items that have passed through
    * this RateAverager over the amount of time passed in this RateAverager.
    *
    * @return the total rate recorded in items per second.
    */
   virtual double getTotalItemsPerSecond();

   /**
    * Gets the total number of items passed through this RateAverager.
    *
    * @return the total number of items passed through this RateAverager.
    */
   virtual uint64_t getTotalItemCount();

   /**
    * Sets the window length in milliseconds. The larger the window, the
    * more accurate an average rate will be over a long period of time,
    * the smaller the window the more accurate an average rate will be
    * over short periods of time.
    *
    * @param length the length of the window in milliseconds >= 2.
    */
   virtual void setWindowLength(uint64_t length);

   /**
    * Gets the window length in milliseconds. The larger the window, the
    * more accurate an average rate will be over a long period of time,
    * the smaller the window the more accurate an average rate will be
    * over short periods of time.
    *
    * @return the length of the window in milliseconds >= 2.
    */
   virtual uint64_t getWindowLength();

   /**
    * Gets an ETA (in seconds) for the given number of items.
    *
    * @param count the item count to get an ETA for.
    * @param current true to use the current rate, false to use the total rate.
    *
    * @return the amount of time (in seconds) until the passed number of
    *         items is reached according to the current rate.
    */
   virtual uint64_t getEta(uint64_t count, bool current = true);

protected:
   /**
    * Updates the current and next windows with a new start time.
    *
    * @param time the new start time for the current window.
    */
   virtual void setWindowStartTimes(uint64_t time);

   /**
    * Updates/moves the windows according to the current time.
    *
    * @param now the current time.
    */
   virtual void updateWindows(uint64_t now);

   /**
    * Gets half of the current window length. Rounds up.
    *
    * @return half of the current window length, rounded up.
    */
   virtual uint64_t getHalfWindowLength();

   /**
    * Updates the current and next windows with a new length.
    *
    * @param length the new window length.
    */
   virtual void updateWindowLengths(uint64_t length);
};

} // end namespace util
} // end namespace db
#endif
