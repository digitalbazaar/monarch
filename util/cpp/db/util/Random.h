/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_Random_H
#define db_util_Random_H

#include <inttypes.h>

#ifdef WIN32
#include "db/rt/Thread.h"
#endif

namespace db
{
namespace util
{

/**
 * The Random class is used to obtain pseudo-random (non-secure) numbers.
 * 
 * @author Dave Longley
 */
class Random
{
protected:
#ifdef WIN32
   /**
    * Stores the thread that seeded this random.
    */
   db::rt::Thread* mSeedThread;
#endif
   
public:
   /**
    * Creates a new Random.
    */
   Random();
   
   /**
    * Destructs this Random.
    */
   virtual ~Random();
   
   /**
    * Gets the next pseudo-random (non-secure) number between "low" and "high".
    * 
    * @param low the lowest possible number to return.
    * @param high the highest possible number to return.
    * 
    * @return a pseduo-random (non-secure) number between "low" and "high". 
    */
   virtual uint64_t next(uint64_t low, uint64_t high);
   
   /**
    * Must be called once at application start up to seed RNG.
    */
   static void seed();
};

} // end namespace util
} // end namespace db
#endif
