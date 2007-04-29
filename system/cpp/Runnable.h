/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Runnable_H
#define Runnable_H

namespace db
{
namespace system
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
    * Runs some implementation specific operation.
    */
   virtual void run() = 0;
};

} // end namespace util
} // end namespace db
#endif
