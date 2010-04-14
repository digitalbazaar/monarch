/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Member.h"

using namespace monarch::rt;
using namespace monarch::validation;

Member::Member(const char* key, const char* errorMessage) :
   Validator(errorMessage),
   mKey(strdup(key))
{
}

Member::~Member()
{
   free(mKey);
}

bool Member::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;

   if(!obj.isNull() && obj->getType() == monarch::rt::Map)
   {
      if(!obj->hasMember(mKey))
      {
         // only add a "." if this is not a root map
         if(context->getDepth() != 0)
         {
            context->pushPath(".");
         }
         context->pushPath(mKey);
         DynamicObject detail =
            context->addError("monarch.validation.MissingField", &obj);
         detail["validator"] = "monarch.validator.Member";
         detail["message"] = (mErrorMessage == NULL) ?
            "A required field has not been specified." :
            mErrorMessage;
         detail["key"] = mKey;
         rval = false;
         context->popPath();
         if(context->getDepth() > 0)
         {
            context->popPath();
         }
      }
   }
   else
   {
      rval = false;
      DynamicObject detail =
         context->addError("monarch.validation.TypeError", &obj);
      detail["validator"] = "monarch.validator.Member";
      detail["message"] = "The given object type must a mapping (Map) type";
   }

   return rval;
}
