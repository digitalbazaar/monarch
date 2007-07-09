/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Runnable_H
#define db_rt_Runnable_H

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
    * @param str the string to populate.
    * 
    * @return a string representation for this Runnable.
    */
   virtual std::string& toString(std::string& str);
};

inline std::string& Runnable::toString(std::string& str)
{
   str = "Runnable";
   return str;
}

} // end namespace rt
} // end namespace db
#endif
