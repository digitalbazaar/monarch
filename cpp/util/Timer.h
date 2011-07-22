/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Timer_H
#define monarch_util_Timer_H

#include <inttypes.h>

namespace monarch
{
namespace util
{

/**
 * The Timer class is used to time things.
 *
 * @author Dave Longley
 */
class Timer
{
protected:
   /**
    * Start time.
    */
   uint64_t mStartTime;

public:
   /**
    * Creates a new Timer.
    */
   Timer();

   /**
    * Destructs this Timer.
    */
   virtual ~Timer();

   /**
    * Starts this Timer.
    *
    * @return the starting time in milliseconds since the epoch.
    */
   virtual uint64_t start();

   /**
    * Gets the time that this timer was started.
    *
    * @return the starting time in milliseconds since the epoch.
    */
   virtual uint64_t getStartTime();

   /**
    * Gets the time that has elapsed since the starting time.
    *
    * @return the time that elapsed in milliseconds.
    */
   virtual uint64_t getElapsedMilliseconds();

   /**
    * Gets the time that has elasped since the starting time.
    *
    * @return the time that elapsed in seconds.
    */
   virtual double getElapsedSeconds();

   /**
    * Gets the time that remains until the given time.
    *
    * @param until the ending time to check, in milliseconds, relative to
    *              the start time.
    *
    * @return the time that remains in milliseconds.
    */
   virtual uint64_t getRemainingMilliseconds(uint64_t until);

   /**
    * Gets the time that remains until the given time.
    *
    * @param until the ending time to check, in milliseconds, relative to
    *              the start time.
    *
    * @return the time that remains in seconds.
    */
   virtual double getRemainingSeconds(uint64_t until);

   /**
    * Starts timing and returns the starting time (in milliseconds since
    * the epoch).
    *
    * @return the starting time in milliseconds.
    */
   static uint64_t startTiming();

   /**
    * Gets the time that has elasped since the passed starting time.
    *
    * @param startTime the time, in milliseconds, when the timer started.
    *
    * @return the time that elapsed in milliseconds.
    */
   static uint64_t getMilliseconds(uint64_t startTime);

   /**
    * Gets the time that has elasped since the passed starting time.
    *
    * @param startTime the time, in milliseconds, when the timer started.
    *
    * @return the time that elapsed in seconds.
    */
   static double getSeconds(uint64_t startTime);
};

} // end namespace util
} // end namespace monarch
#endif
