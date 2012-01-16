/*
 * Copyright (c) 2012 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Contains.h"

#include "monarch/rt/DynamicObjectIterator.h"

using namespace monarch::rt;
using namespace monarch::validation;

Contains::Contains(DynamicObject& object, const char* errorMessage) :
   Validator(errorMessage),
   mObject(object)
{
}

Contains::~Contains()
{
}

bool Contains::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = false;

   // check if objects are equal
   rval = (obj == mObject);

   // if not equal, check if target contains the validation object
   if(!rval && !obj.isNull() &&
      (obj->getType() == Array || obj->getType() == Map))
   {
      DynamicObjectIterator i = obj.getIterator();
      while(!rval && i->hasNext())
      {
         DynamicObject& next = i->next();
         rval = (next == mObject);
      }
   }

   if(!rval)
   {
      DynamicObject detail =
         context->addError("monarch.validation.NotFound", &obj);
      detail["validator"] = "monarch.validator.Contains";
      detail["expectedValue"] = mObject;
      detail["message"] = mErrorMessage ? mErrorMessage :
         "The input object was not equal to or found in the validator.";
   }

   if(rval)
   {
      context->addSuccess();
   }

   return rval;
}
