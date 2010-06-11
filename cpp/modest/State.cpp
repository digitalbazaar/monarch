/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/State.h"

#include <cstdlib>

using namespace std;
using namespace monarch::modest;

State::State()
{
}

State::~State()
{
   // delete all state variables
   for(map<const char*, Variable*, VarNameComparator>::iterator i =
       mVarTable.begin(); i != mVarTable.end(); ++i)
   {
      // free name
      free((char*)i->first);

      // free variable
      freeVariable(i->second);
   }
}

State::Variable* State::getVariable(const char* name)
{
   Variable* rval = NULL;

   map<const char*, Variable*, VarNameComparator>::iterator i =
      mVarTable.find(name);
   if(i != mVarTable.end())
   {
      rval = i->second;
   }

   return rval;
}

State::Variable* State::createVariable(const char* name, Variable::Type type)
{
   // check for an existing variable
   Variable* var = getVariable(name);
   if(var == NULL)
   {
      // create new variable
      var = new Variable();
      var->type = type;

      // create string as appropriate
      if(type == Variable::String)
      {
         var->s = new string();
      }

      // store variable in table
      mVarTable.insert(make_pair(strdup(name), var));
   }

   if(var->type != type)
   {
      // allocate or delete string as appropriate
      if(type == Variable::String)
      {
         var->s = new string();
      }
      else if(var->type == Variable::String)
      {
         delete var->s;
      }
   }

   return var;
}

void State::freeVariable(Variable* var)
{
   // delete string if applicable
   if(var->type == Variable::String)
   {
      delete var->s;
   }

   // delete variable
   delete var;
}

void State::setBoolean(const char* name, bool value)
{
   // create the variable and set its value
   Variable* var = createVariable(name, Variable::Boolean);
   var->b = value;
}

bool State::getBoolean(const char* name, bool& value)
{
   bool rval = false;

   Variable* var = getVariable(name);
   if(var != NULL && var->type == Variable::Boolean)
   {
      value = var->b;
      rval = true;
   }

   return rval;
}

void State::setInteger(const char* name, int32_t value)
{
   // create the variable and set its value
   Variable* var = createVariable(name, Variable::Integer);
   var->i = value;
}

bool State::increaseInteger(const char* name, int32_t amount, int32_t* value)
{
   bool rval;

   // get variable
   Variable* var = getVariable(name);
   if(var != NULL && var->type == Variable::Integer)
   {
      // increase variable
      var->i += amount;
      if(value != NULL)
      {
         *value = var->i;
      }

      rval = true;
   }

   return rval;
}

bool State::getInteger(const char* name, int32_t& value)
{
   bool rval = false;

   Variable* var = getVariable(name);
   if(var != NULL && var->type == Variable::Integer)
   {
      value = var->i;
      rval = true;
   }

   return rval;
}

bool State::getIntegerDifference(
   const char* name1, const char* name2, int32_t& value)
{
   bool rval = false;

   // get integer 1 and integer 2
   int32_t first, second;
   if(getInteger(name1, first) && getInteger(name2, second))
   {
      value = first - second;
      rval = true;
   }

   return rval;
}

void State::setString(const char* name, const string& value)
{
   // create the variable and set its value
   Variable* var = createVariable(name, Variable::String);
   var->s->assign(value);
}

bool State::getString(const char* name, string& value)
{
   bool rval = false;

   Variable* var = getVariable(name);
   if(var != NULL && var->type == Variable::String)
   {
      value = *(var->s);
      rval = true;
   }

   return rval;
}

void State::removeVariable(const char* name)
{
   map<const char*, Variable*, VarNameComparator>::iterator i =
      mVarTable.find(name);
   if(i != mVarTable.end())
   {
      // free name
      free((char*)i->first);

      // free variable
      freeVariable(i->second);

      // remove variable from table
      mVarTable.erase(i);
   }
}
