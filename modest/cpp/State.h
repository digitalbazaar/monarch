/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_State_H
#define db_modest_State_H

#include <map>

#include "Object.h"
#include "ImmutableState.h"

namespace db
{
namespace modest
{

typedef struct StateNameComparator
{
   /**
    * Compares two null-terminated strings, returning true if the first is
    * less than the second, false if not.
    * 
    * @param s1 the first string.
    * @param s2 the second string.
    * 
    * @return true if the s1 < s2, false if not.
    */
   bool operator()(const char* s1, const char* s2) const
   {
      return strcmp(s1, s2) < 0;
   }
};

/**
 * A StateVariable is a boolean, 32-bit signed integer, or a string.
 */
typedef struct StateVariable
{
   /**
    * The type of variable this is, 1. bool, 2. int, 3. string.
    */
   int type;
   
   /**
    * The actual value of this variable.
    */
   union
   {
      bool b;
      int i;
      std::string* s;
   };
};

/**
 * A State maintains the current information about a Modest Engine. It can
 * be modified by an Operation.
 * 
 * @author Dave Longley
 */
class State : public virtual db::rt::Object, public ImmutableState
{
protected:
   /**
    * The variable table.
    */
   std::map<const char*, StateVariable*, StateNameComparator> mVarTable;
   
   /**
    * Gets an existing StateVariable by its name.
    * 
    * @param name the name of the StateVariable to retrieve.
    * 
    * @return an existing StateVariable or NULL.
    */
   virtual StateVariable* getVariable(const char* name);
   
   /**
    * Creates a new StateVariable with the given name if it does not
    * exist.
    * 
    * @param name the name of the StateVariable to create.
    * @param type the type of variable (1 = bool, 2 = int, 3 = string).
    * 
    * @return the StateVariable.
    */
   virtual StateVariable* createVariable(const char* name, int type);
   
public:
   /**
    * Creates a new State.
    */
   State();
   
   /**
    * Destructs this State.
    */
   virtual ~State();
   
   /**
    * Sets a boolean in this State.
    * 
    * @param name the name of the boolean to set.
    * @param value the value of the boolean.
    */
   virtual void setBoolean(const char* name, bool value);
   
   /**
    * Gets a boolean from this State by its name.
    * 
    * @param name the name of the boolean to retrieve.
    * @param value a boolean to set to the value of the boolean.
    * 
    * @return true if the boolean exists, false if not.
    */
   virtual bool getBoolean(const char* name, bool& value);
   
   /**
    * Sets a 32-bit signed integer in this State.
    * 
    * @param name the name of the integer to set.
    * @param value the value of the integer.
    */
   virtual void setInteger(const char* name, int value);
   
   /**
    * Gets a 32-bit signed integer from this State by its name.
    * 
    * @param name the name of the integer to retrieve.
    * @param value an integer to set to the value of the integer.
    * 
    * @return true if the integer exists, false if not.
    */
   virtual bool getInteger(const char* name, int& value);
   
   /**
    * Sets a string in this State. The string will be copied to this
    * State's internal storage.
    * 
    * @param name the name of the string to set.
    * @param value the value of the string.
    */
   virtual void setString(const char* name, const std::string& value);
   
   /**
    * Gets a string from this State by its name.
    * 
    * @param name the name of the string to retrieve.
    * @param value a string to set to the value of the string.
    * 
    * @return true if the string exists, false if not.
    */
   virtual bool getString(const char* name, std::string& value);
   
   /**
    * Removes a variable from this state by its name.
    * 
    * @param name the name of the variable to remove.
    */
   virtual void removeVariable(const char* name);
};

} // end namespace modest
} // end namespace db
#endif
