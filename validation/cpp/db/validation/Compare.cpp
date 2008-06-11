/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Compare.h"

using namespace db::rt;
using namespace db::validation;

Compare::Compare(const char* key0, const char* key1, const char* errorMessage) :
   Validator(errorMessage),
   mKey0(key0),
   mKey1(key1)
{
}

Compare::~Compare()
{
}

bool Compare::isValid(
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;
   
   if(obj.isNull() || obj->getType() != Map)
   {
      rval = false;
      DynamicObject detail =
         context->addError("db.validation.TypeError");
      detail["validator"] = "db.validator.Compare";
      detail["message"] = "Object not a Map!";
   }
   else
   {
      rval = obj->hasMember(mKey0) &&
         obj->hasMember(mKey1) &&
         (obj[mKey0] == obj[mKey1]);
      
      if(!rval)
      {
         if(context->getDepth() != 0)
         {
            context->pushPath(".");
         }
         
         context->pushPath(mKey1);
         DynamicObject detail =
            context->addError("db.validation.CompareFailure", &obj);
         detail["validator"] = "db.validator.Compare";
         detail["message"] =
            mErrorMessage ? mErrorMessage : "Comparison failure!";
         detail["key0"] = mKey0;
         detail["key1"] = mKey1;
         detail["expectedValue"] = obj[mKey0];
         context->popPath();
         
         if(context->getDepth() == 1)
         {
            context->popPath();
         }
      }
   }
   
   return rval;
}
