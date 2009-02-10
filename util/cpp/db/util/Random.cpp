/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Random.h"

#include "db/rt/System.h"

#include <stdlib.h>
#include <iostream>

using namespace db::rt;
using namespace db::util;

void Random::seed()
{
   uint64_t time = System::getCurrentMilliseconds();
   Random::seed((unsigned int)(time & 0xFFFFFFFF));
}

void Random::seed(unsigned int value)
{
#ifdef WIN32
   std::srand(value);
#else
   srandom(value);
#endif
}

uint64_t Random::next(uint64_t low, uint64_t high)
{
#ifdef WIN32
   unsigned long num = std::rand();
   // get a random number between 1 and 1000000000
   return low + (uint64_t)((long double)high * (num / (RAND_MAX + 1.0)));
#else
   // get a random number between 1 and 1000000000
   return low + (uint64_t)((long double)high * (random() / (RAND_MAX + 1.0)));
#endif
}
