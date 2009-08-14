/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/validation/In.h"

#include "db/rt/DynamicObjectIterator.h"

using namespace db::rt;
using namespace db::validation;

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
            rval = obj == member;
         }
         break;
      }
      case Map:
         rval =
            !obj.isNull() &&
            obj->getType() == String &&
            mContents->hasMember(obj->getString());
         break;
      default:
         rval = false;
         valid = false;
         DynamicObject detail =
            context->addError("db.validation.InternalError");
         detail["validator"] = "db.validator.In";
         detail["message"] = "There was an error when determining the type for "
            "the input object to the validator. The input object type is "
            "unknown.";
         break;
   }

   if(!rval && valid)
   {
      DynamicObject detail = context->addError("db.validation.NotFound", &obj);
      detail["validator"] = "db.validator.In";
      detail["expectedValues"] = mContents;
      detail["message"] = mErrorMessage ? mErrorMessage :
         "There was an error in the validator when retrieving a value "
         "for the input object.";
   }

   if(rval)
   {
      context->addSuccess();
   }

   return rval;
}
