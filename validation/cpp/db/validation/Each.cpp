/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Each.h"
#include "db/rt/DynamicObjectIterator.h"

using namespace db::rt;
using namespace db::validation;

Each::Each(Validator* validator) :
   mValidator(validator)
{
}

Each::~Each()
{
   delete mValidator;
}

bool Each::isArrayValid(
   DynamicObject& obj,
   DynamicObject* state,
   std::vector<const char*>* path)
{
   bool rval = true;
   
   DynamicObjectIterator doi = obj.getIterator();
   
   int i = -1;
   while(doi->hasNext())
   {
      DynamicObject& member = doi->next();
      i++;
      
      // add [#] indexing to path even if at root
      char idx[23];
      snprintf(idx, 23, "[%d]", i);
      path->push_back(idx);
      rval &= mValidator->isValid(member, state, path);
      path->pop_back();
   }
   
   return rval;
}

bool Each::isMapValid(
   DynamicObject& obj,
   DynamicObject* state,
   std::vector<const char*>* path)
{
   bool rval = true;
   
   DynamicObjectIterator doi = obj.getIterator();
   
   int i = -1;
   while(doi->hasNext())
   {
      DynamicObject& member = doi->next();
      i++;
      
      // only add a "." if this is not a root map
      if(path->size() != 0)
      {
         path->push_back(".");
      }
      path->push_back(doi->getName());
      rval &= mValidator->isValid(member, state, path);
      path->pop_back();
      if(path->size() == 1)
      {
         path->pop_back();
      }
   }

   return rval;
}

bool Each::isValid(
   DynamicObject& obj,
   DynamicObject* state,
   std::vector<const char*>* path)
{
   bool rval = true;
   bool madePath = false;
   
   // create a path if there isn't one yet
   if(path == NULL)
   {
      madePath = true;
      path = new std::vector<const char*>;
   }

   switch(obj->getType())
   {
      case Array:
         rval = isArrayValid(obj, state, path);
         break;
      case Map:
         rval = isMapValid(obj, state, path);
         break;
      default:
         rval = false;
         DynamicObject detail = addError(path, "db.validation.TypeError");
         char exp[100];
         snprintf(exp, 100, "%s | %s",
            DynamicObject::descriptionForType(Map),
            DynamicObject::descriptionForType(Array));
         detail["expectedType"] = exp;
         break;
   }

   if(madePath)
   {
      delete path;
   }
   
   return rval;
}
