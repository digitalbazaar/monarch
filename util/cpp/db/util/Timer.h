/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_Timer_H
#define db_util_Timer_H

#include <inttypes.h>

namespace db
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
public:
   /**
    * Creates a new Timer.
    */
   Timer() {};
   
   /**
    * Destructs this Timer.
    */
   ~Timer() {};
   
   /**
    * Starts this Timer.
    */
   virtual void start();
   
   /**
    * Stops this Timer.
    */
   virtual void stop();
   
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
} // end namespace db
#endif
