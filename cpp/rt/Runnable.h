/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_Runnable_H
#define monarch_rt_Runnable_H

#include "monarch/rt/Collectable.h"

namespace monarch
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

// define a counted reference Runnable type
typedef Collectable<Runnable> RunnableRef;

} // end namespace rt
} // end namespace monarch
#endif
