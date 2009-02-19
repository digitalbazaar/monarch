/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_modest_ImmutableState_H
#define db_modest_ImmutableState_H

#include <cstring>
#include <string>

namespace db
{
namespace modest
{

/**
 * An ImmutableState is a wrapper for a State that protects it from mutation.
 * 
 * @author Dave Longley
 */
class ImmutableState
{
public:
   /**
    * Creates a new ImmutableState.
    */
   ImmutableState() {};
   
   /**
    * Destructs this ImmutableState.
    */
   virtual ~ImmutableState() {};
   
   /**
    * Gets a boolean from this State by its name.
    * 
    * @param name the name of the boolean to retrieve.
    * @param value a boolean to set to the value of the boolean.
    * 
    * @return true if the boolean exists, false if not.
    */
   virtual bool getBoolean(const char* name, bool& value) = 0;
   
   /**
    * Gets a 32-bit signed integer from this State by its name.
    * 
    * @param name the name of the integer to retrieve.
    * @param value an integer to set to the value of the integer.
    * 
    * @return true if the integer exists, false if not.
    */
   virtual bool getInteger(const char* name, int32_t& value) = 0;
   
   /**
    * Gets the difference between two 32-bit signed integers from this State.
    * 
    * @param name1 the name of the first integer to retrieve.
    * @param name2 the name of the second integer to retrieve.
    * @param value the difference between the two integers (name1 - name2).
    * 
    * @return true if both integers exist, false if not.
    */
   virtual bool getIntegerDifference(
      const char* name1, const char* name2, int32_t& value) = 0;
   
   /**
    * Gets a string from this State by its name.
    * 
    * @param name the name of the string to retrieve.
    * @param value a string to set to the value of the string.
    * 
    * @return true if the string exists, false if not.
    */
   virtual bool getString(const char* name, std::string& value) = 0;
};

} // end namespace modest
} // end namespace db
#endif
