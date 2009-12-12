/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_System_H
#define db_rt_System_H

#include "monarch/rt/TimeFunctions.h"

#include <inttypes.h>

namespace db
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
};

} // end namespace rt
} // end namespace db
#endif
