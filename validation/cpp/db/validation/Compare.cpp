/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Compare.h"

using namespace db::rt;
using namespace db::validation;

Compare::Compare(const char* key0, const char* key1) :
   mKey0(key0),
   mKey1(key1)
{
}

Compare::~Compare()
{
}

bool Compare::isValid(
   db::rt::DynamicObject& obj,
   db::rt::DynamicObject* state,
   std::vector<const char*>* path)
{
   bool madePath = false;
   bool rval = obj[mKey0] == obj[mKey1];

   if(!rval)
   {
      if(!path)
      {
         madePath = true;
         path = new std::vector<const char*>;
      }
      else
      {
         path->push_back(".");
      }
      
      path->push_back(mKey1);
      DynamicObject detail = addError(path, "db.validation.CompareFailure");
      detail["expectedValue"] = obj[mKey0];
      path->pop_back();
      
      if(madePath)
      {
         delete path;
      }
      else
      {
         path->pop_back();
      }
   }
   return rval;
}
