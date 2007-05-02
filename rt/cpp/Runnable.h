/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Runnable_H
#define Runnable_H

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

} // end namespace rt
} // end namespace db
#endif
