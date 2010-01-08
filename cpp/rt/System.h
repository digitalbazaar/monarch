/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_System_H
#define monarch_rt_System_H

#include "monarch/rt/TimeFunctions.h"

#include <inttypes.h>

namespace monarch
{
namespace rt
{

/**
 * The System class has methods for obtaining information or interacting
 * with the system a program is running on.
 *
 * @author Dave Longley
 */
class System
{
public:
   /**
    * Gets the current time in milliseconds.
    *
    * @return the current time in milliseconds.
    */
   static uint64_t getCurrentMilliseconds();

   /**
    * Gets the number of cores/cpus.
    *
    * @return the number of cores/cpus.
    */
   static uint32_t getCpuCoreCount();
};

} // end namespace rt
} // end namespace monarch
#endif
