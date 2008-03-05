/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
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
         }
         break;
      case Map:
         rval =
            obj->getType() == String &&
            mContents->hasMember(obj->getString());
         break;
      default:
         rval = false;
         DynamicObject detail =
            context->addError("db.validation.InternalError");
         detail["message"] = "Invalid validator!";
         break;
   }
   
   if(!rval)
   {
      DynamicObject detail = context->addError("db.validation.NotFound");
      detail["message"] = mErrorMessage ? mErrorMessage : "Value not found!";
   }
   
   return rval;
}
