/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_Random_H
#define db_util_Random_H

#include <inttypes.h>

namespace db
{
namespace util
{

/**
 * The Random class is used to obtain random numbers.
 * 
 * @author Dave Longley
 */
class Random
{
private:
   /**
    * Creates a new Random.
    */
   Random() {};
   
public:
   /**
    * Destructs this Random.
    */
   virtual ~Random() {};
   
   /**
    * Gets the next random number between "low" and "high".
    * 
    * @param low the lowest possible number to return.
    * @param high the highest possible number to return.
    * 
    * @return a random number between "low" and "high". 
    */
   static uint64_t next(uint64_t low, uint64_t high);
};

} // end namespace util
} // end namespace db
#endif
