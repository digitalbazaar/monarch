/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_RateAverager_H
#define db_util_RateAverager_H

#include "db/util/TimeWindow.h"

namespace db
{
namespace util
{

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
class RateAverager
{
protected:
   /**
    * Whether or not this RateAverager is currently running.
    */
   bool mRunning;
   
   /**
    * The time (in milliseconds) at which this RateAverager started.
    */
   uint64_t mStartTime;
   
   /**
    * The time (in milliseconds) at which this RateAverager stopped.
    */
   uint64_t mStopTime;
   
   /**
    * The time (in milliseconds) that has passed since this RateAverager
    * started.
    */
   uint64_t mTimePassed;
   
   /**
    * The last time that time (in milliseconds) that a rate was added to this
    * window. This is used to help convenience methods that can add the amount
    * of time (in milliseconds) since the last addition of time to this
    * window.
    * 
    * Uses absolute time only.
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
    * The total item count since the RateAverager started.
    */
   uint64_t mTotalItemCount;
   
   /**
    * A lock for synchronizing this rate averager.
    */
   db::rt::ExclusiveLock mLock;
   
   /**
    * Resets the calculated data and times for this RateAverager.
    */
   virtual void reset();
   
   /**
    * Sets the time (in milliseconds) at which this RateAverager started.
    * 
    * The start time that this RateAverager uses can be relative or absolute.
    * 
    * @param time the time (in milliseconds) at which this RateAverager started.
    */
   virtual void setStartTime(uint64_t time);
   
   /**
    * Sets the time (in milliseconds) at which this RateAverager stopped.
    * 
    * The stop time that this RateAverager uses can be relative or absolute.
    * 
    * @param time the time (in milliseconds) at which this RateAverager stopped.
    */
   virtual void setStopTime(uint64_t time);
   
   /**
    * Updates the current and next windows with a new start time.
    * 
    * @param time the new start time for the current window.
    */
   virtual void setWindowStartTimes(uint64_t time);
   
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
   
   /**
    * Moves the current window to the next window and resets the next
    * window.
    */
   virtual void moveCurrentWindow();
   
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
    * Starts this RateAverager if it hasn't already started. When a
    * RateAverager starts, it clears all of its previously calculated data.
    * 
    * @param time the start time to use (in milliseconds) -- can be absolute
    *             or relative.
    */
   virtual void start(uint64_t time = 0);
   
   /**
    * Stops this RateAverager if it is running. The calculated data for this
    * RateAverager will not be cleared when the RateAverager stops, it will
    * be cleared if start() is called again afterwards.
    * 
    * Uses a stop time of getCurrentTime().
    */
   virtual void stop();   
   
   /**
    * Stops this RateAverager if it is running. The calculated data for this
    * RateAverager will not be cleared when the RateAverager stops, it will
    * be cleared if start() is called again afterwards.
    * 
    * @param time the stop time to use (in milliseconds) -- can be absolute
    *             or relative.
    */
   virtual void stop(uint64_t time);
   
   /**
    * Restarts this rate averager. Calls stop() and then start().
    * 
    * Uses a stop time of getCurrentTime() and a start time of 0.
    */
   virtual void restart();
   
   /**
    * Restarts this rate averager. Calls stop() and then start().
    * 
    * @param stopTime the stop time to use (in milliseconds) -- can be absolute
    *                 or relative.
    * @param startTime the start time to use (in milliseconds) -- can be
    *                  absolute or relative.
    */
   virtual void restart(uint64_t stopTime, uint64_t startTime);
   
   /**
    * Gets whether or not this RateAverager is running.
    * 
    * @return true if this RateAverager is running, false if not.
    */
   virtual bool isRunning();
   
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
   virtual void addRate(uint64_t count);   
   
   /**
    * Adds a new rate to this RateAverager. Increases the item count and
    * the amount of time passed for the current window in this RateAverager.
    * 
    * @param count the amount of items to increase the item count by.
    * @param interval the interval overwhich the item count increased
    *                 (in milliseconds).
    */
   virtual void addRate(uint64_t count, uint64_t interval);
   
   /**
    * Gets the time (in milliseconds) at which this RateAverager started. If
    * this RateAverager has not started, 0 is returned.
    * 
    * @return the time (in milliseconds) at which this RateAverager started.
    */
   virtual uint64_t getStartTime();
   
   /**
    * Gets the time (in milliseconds) at which this RateAverager stopped. If
    * this RateAverager has not stopped, 0 is returned.
    * 
    * @return the time (in milliseconds) at which this RateAverager stopped.
    */
   virtual uint64_t getStopTime();
   
   /**
    * Gets the current time in this RateAverager. The current time in this
    * RateAverager is the start time plus the time passed.
    * 
    * @return the current time in this RateAverager.
    */
   virtual uint64_t getCurrentTime();
   
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
   virtual uint64_t getTimePassed();
   
   /**
    * Gets the current rate in items per second. The current rate is the
    * average rate in the current time window.
    * 
    * If the RateAverager has not started, then 0 is returned.
    * 
    * @return the current rate in items per second.
    */
   virtual double getCurrentRate();
   
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
   virtual double getTotalRate();
   
   /**
    * Gets the total item count since the RateAverager started.
    * 
    * @return the total item count since the RateAverager started.
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
};

} // end namespace util
} // end namespace db
#endif
