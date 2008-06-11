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
   ValidatorContext* context)
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
      context->pushPath(idx);
      bool objValid = mValidator->isValid(member, context);
      // seperate var to avoid short circuit and ensure all indexes tested
      rval = rval && objValid;
      context->popPath();
   }
   
   return rval;
}

bool Each::isMapValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;
   
   DynamicObjectIterator doi = obj.getIterator();
   
   while(doi->hasNext())
   {
      DynamicObject& member = doi->next();
      
      // only add a "." if this is not a root map
      if(context->getDepth() != 0)
      {
         context->pushPath(".");
      }
      context->pushPath(doi->getName());
      bool objValid = mValidator->isValid(member, context);
      // seperate var to avoid short circuit and ensure all keys tested
      rval = rval && objValid;
      context->popPath();
      if(context->getDepth() == 1)
      {
         context->popPath();
      }
   }

   return rval;
}

bool Each::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;
   
   if(obj.isNull() || !(obj->getType() == Array || obj->getType() == Map))
   {
      rval = false;
      DynamicObject detail =
         context->addError("db.validation.TypeError");
      char exp[100];
      snprintf(exp, 100, "%s | %s",
         DynamicObject::descriptionForType(Map),
         DynamicObject::descriptionForType(Array));
      detail["expectedType"] = exp;
   }
   else
   {
      switch(obj->getType())
      {
         case Array:
            rval = isArrayValid(obj, context);
            break;
         case Map:
            rval = isMapValid(obj, context);
            break;
         default:
            break;
      }
   }

   return rval;
}
