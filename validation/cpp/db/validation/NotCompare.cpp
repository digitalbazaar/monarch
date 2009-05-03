/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/NotCompare.h"

using namespace db::rt;
using namespace db::validation;

NotCompare::NotCompare(
   const char* key0, const char* key1, const char* errorMessage) :
   Validator(errorMessage),
   mKey0(key0),
   mKey1(key1)
{
}

NotCompare::~NotCompare()
{
}

bool NotCompare::isValid(
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;
   
   if(obj.isNull() || obj->getType() != Map)
   {
      rval = false;
      DynamicObject detail =
         context->addError("db.validation.TypeError");
      detail["validator"] = "db.validator.NotCompare";
      detail["message"] = "The given object type must a mapping (Map) type";
   }
   else
   {
      rval = obj->hasMember(mKey0) &&
         obj->hasMember(mKey1) &&
         (obj[mKey0] != obj[mKey1]);
      
      if(!rval)
      {
         if(context->getDepth() != 0)
         {
            context->pushPath(".");
         }
         
         context->pushPath(mKey1);
         DynamicObject detail =
            context->addError("db.validation.NotCompareFailure", &obj);
         detail["validator"] = "db.validator.NotCompare";
         detail["message"] =
            mErrorMessage ? mErrorMessage : 
               "The two objects that were given are required "
               "to be different in some way, but they are equivalent.";
         detail["key0"] = mKey0;
         detail["key1"] = mKey1;
         detail["expectedValue"] = obj[mKey0];
         context->popPath();
         
         if(context->getDepth() == 1)
         {
            context->popPath();
         }
      }
      else
      {
         context->addSuccess();
      }
   }
   
   return rval;
}
