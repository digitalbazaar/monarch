/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Runnable_H
#define db_rt_Runnable_H

#include "db/rt/Collectable.h"

namespace db
{
namespace rt
{

/**
 * An object is Runnable if it implements the run() method.
 *
 * @author Dave Longley
 */
class Runnable
{
public:
   /**
    * Constructs this Runnable.
    */
   Runnable() {};
   
   /**
    * Destructs this Runnable.
    */
   virtual ~Runnable() {};
   
   /**
    * Runs some implementation specific operation.
    */
   virtual void run() = 0;
};

// define a collectable Runnable
typedef Collectable<Runnable> CollectableRunnable;

} // end namespace rt
} // end namespace db
#endif
