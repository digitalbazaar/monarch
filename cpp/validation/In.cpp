/*
 * Copyright (c) 2008-2012 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/In.h"

#include "monarch/rt/DynamicObjectIterator.h"

using namespace monarch::rt;
using namespace monarch::validation;

In::In(DynamicObject& contents, const char* errorMessage) :
   Validator(errorMessage),
   mContents(contents)
{
}

In::~In()
{
}

bool In::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = false;
   bool valid = true;

   switch(mContents->getType())
   {
      case Array:
      {
         DynamicObjectIterator doi = mContents.getIterator();
         while(!rval && doi->hasNext())
         {
            DynamicObject& member = doi->next();
            rval = (obj == member);
         }
         break;
      }
      case Map:
      {
         rval =
            !obj.isNull() &&
            obj->getType() == String &&
            mContents->hasMember(obj->getString());
         break;
      }
      default:
         rval = false;
         valid = false;
         DynamicObject detail = context->addError(
            "monarch.validation.InternalError");
         detail["validator"] = "monarch.validator.In";
         detail["message"] = "The content object is not a container so it "
            "cannot possibly hold any given object.";
         break;
   }

   if(!rval && valid)
   {
      DynamicObject detail = context->addError(
         "monarch.validation.NotFound", &obj);
      detail["validator"] = "monarch.validator.In";
      detail["expectedValues"] = mContents;
      detail["message"] = mErrorMessage ? mErrorMessage :
         "The input object was not found in the content object.";
   }

   if(rval)
   {
      context->addSuccess();
   }

   return rval;
}
