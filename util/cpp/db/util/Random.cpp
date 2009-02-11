/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Random.h"

#include "db/rt/System.h"

#include <stdlib.h>
#include <time.h>

using namespace db::rt;
using namespace db::util;

Random::Random()
{
#ifdef WIN32
   mSeedThread = NULL;
#endif
}

Random::~Random()
{
}

uint64_t Random::next(uint64_t low, uint64_t high)
{
#ifdef WIN32
   // random is per-thread in windows
   Thread* t = Thread::currentThread();
   if(mSeedThread != t)
   {
      // will now be seeded on this thread
      mSeedThread = t;
      
      // add thread ID to make seed more unique to this thread
      uint64_t value = (unsigned long)t->getId().p;
      value += System::getCurrentMilliseconds();
      srand((unsigned int)(value & 0xFFFFFFFF) + time(NULL));
   }
   
   // get a random number between low and high
   return low + (uint64_t)((long double)high * (rand() / (RAND_MAX + 1.0)));
#else
   // get a random number between low and high
   return low + (uint64_t)((long double)high * (random() / (RAND_MAX + 1.0)));
#endif
}

void Random::seed()
{
#ifndef WIN32
   uint64_t value = System::getCurrentMilliseconds();
   srandom((unsigned int)(value & 0xFFFFFFFF) + time(NULL));
#endif
}
