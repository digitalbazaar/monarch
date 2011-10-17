/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Compare.h"

using namespace monarch::rt;
using namespace monarch::validation;

Compare::Compare(const char* key0, const char* key1, const char* errorMessage) :
   Validator(errorMessage),
   mKey0(strdup(key0)),
   mKey1(strdup(key1))
{
}

Compare::~Compare()
{
   free(mKey0);
   free(mKey1);
}

bool Compare::isValid(
   monarch::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;

   if(obj.isNull() || obj->getType() != Map)
   {
      rval = false;
      DynamicObject detail =
         context->addError("monarch.validation.TypeError");
      detail["validator"] = "monarch.validator.Compare";
      detail["message"] = "The given object type must be a mapping (Map) type.";
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
            context->addError("monarch.validation.CompareFailure", &obj);
         detail["validator"] = "monarch.validator.Compare";
         detail["message"] =
            mErrorMessage ? mErrorMessage : \
               "The two objects that were compared are required to be "
               "equivalent, but they are different.";
         detail["key0"] = mKey0;
         detail["key1"] = mKey1;
         detail["expectedValue"] = obj[mKey0];
         context->popPath();

         if(context->getDepth() > 1)
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
