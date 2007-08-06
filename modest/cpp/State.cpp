/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "State.h"

using namespace std;
using namespace db::modest;

State::State()
{
}

State::~State()
{
   // delete all state variables
   for(map<const char*, StateVariable*, StateNameComparator>::iterator i =
       mVarTable.begin(); i != mVarTable.end(); i++)
   {
      // delete name
      delete [] i->first;
      
      // get variable
      StateVariable* var = i->second;
      
      // delete string if applicable
      if(var->type == 3)
      {
         delete var->s;
      }
      
      // delete variable
      delete var;
   }
}

StateVariable* State::getVariable(const char* name)
{
   StateVariable* rval = NULL;
   
   map<const char*, StateVariable*, StateNameComparator>::iterator i =
      mVarTable.find(name);
   if(i != mVarTable.end())
   {
      rval = i->second;
   }
   
   return rval;
}

StateVariable* State::createVariable(const char* name, int type)
{
   // check for an existing variable
   StateVariable* var = getVariable(name);
   if(var == NULL)
   {
      // create new variable
      var = new StateVariable();
      var->type = type;
      
      // create string as appropriate
      if(type == 3)
      {
         var->s = new string();
      }
      
      // store variable in table
      char* str = new char[strlen(name) + 1];
      strcpy(str, name);
      mVarTable.insert(make_pair(str, var));
   }
   
   if(var->type != type)
   {
      // allocate or delete string as appropriate
      if(type == 3)
      {
         var->s = new string();
      }
      else if(var->type == 3)
      {
         delete var->s;
      }
   }
   
   return var;
}

void State::setBoolean(const char* name, bool value)
{
   // create the variable and set its value
   StateVariable* var = createVariable(name, 1);
   var->b = value;
}

bool State::getBoolean(const char* name, bool& value)
{
   bool rval = false;
   
   StateVariable* var = getVariable(name);
   if(var != NULL && var->type == 1)
   {
      value = var->b;
      rval = true;
   }
   
   return rval;
}

void State::setInteger(const char* name, int value)
{
   // create the variable and set its value
   StateVariable* var = createVariable(name, 2);
   var->i = value;
}

bool State::getInteger(const char* name, int& value)
{
   bool rval = false;
   
   StateVariable* var = getVariable(name);
   if(var != NULL && var->type == 2)
   {
      value = var->i;
      rval = true;
   }
   
   return rval;
}

void State::setString(const char* name, const string& value)
{
   // create the variable and set its value
   StateVariable* var = createVariable(name, 3);
   var->s->assign(value);
}

bool State::getString(const char* name, string& value)
{
   bool rval = false;
   
   StateVariable* var = getVariable(name);
   if(var != NULL && var->type == 3)
   {
      value = *(var->s);
      rval = true;
   }
   
   return rval;
}

void State::removeVariable(const char* name)
{
   map<const char*, StateVariable*, StateNameComparator>::iterator i =
      mVarTable.find(name);
   if(i != mVarTable.end())
   {
      // delete name
      delete [] i->first;
      
      // get variable
      StateVariable* var = i->second;
      
      // delete string if applicable
      if(var->type == 3)
      {
         delete var->s;
      }
      
      // delete variable
      delete var;
      
      // remove variable from table
      mVarTable.erase(i);
   }
}
