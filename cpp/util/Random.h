/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_util_Random_H
#define db_util_Random_H

#include <inttypes.h>

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
public:
   /**
    * Destructs this Random.
    */
   virtual ~Random() {};

   /**
    * Must be called once at application start up to seed RNG.
    */
   static void seed();

   /**
    * Gets the next pseudo-random (non-secure) number between "low" and "high".
    *
    * @param low the lowest possible number to return.
    * @param high the highest possible number to return.
    *
    * @return a pseduo-random (non-secure) number between "low" and "high".
    */
   static uint64_t next(uint64_t low, uint64_t high);

protected:
   /**
    * Creates a new Random.
    */
   Random() {};
};

} // end namespace util
} // end namespace db
#endif
