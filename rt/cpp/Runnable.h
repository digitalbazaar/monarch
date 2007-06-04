/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Runnable_H
#define Runnable_H

#include <string>

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
   
   /**
    * Gets a string representation for this Runnable.
    * 
    * @return a string representation for this Runnable.
    */
   virtual void toString(std::string& str);
};

inline void Runnable::toString(std::string& str)
{
   str = "Runnable";
}

} // end namespace rt
} // end namespace db
#endif
