/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Random.h"

#include "db/rt/System.h"

#include <stdlib.h>
#include <time.h>

using namespace db::rt;
using namespace db::util;

void Random::seed()
{
#ifndef WIN32
   // only do actual seeding on linux, each thread is seeded on windows
   // as rand() is per-thread on win32
   uint64_t value = System::getCurrentMilliseconds();
   srandom((unsigned int)(value & 0xFFFFFFFF) + time(NULL));
#endif
}

uint64_t Random::next(uint64_t low, uint64_t high)
{
   // get a random number between low and high
#ifdef WIN32
   return low + (uint64_t)((long double)high * (rand() / (RAND_MAX + 1.0)));
#else
   return low + (uint64_t)((long double)high * (random() / (RAND_MAX + 1.0)));
#endif
}
