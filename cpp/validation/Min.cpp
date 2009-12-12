/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Min.h"

using namespace monarch::rt;
using namespace monarch::validation;

Min::Min(int size, const char* errorMessage) :
   Validator(errorMessage),
   mSize(size)
{
}

Min::~Min()
{
}

bool Min::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = (!obj.isNull() && obj->length() >= mSize);

   if(!rval)
   {
      DynamicObject detail = context->addError("monarch.validation.MinError", &obj);
      detail["validator"] = "monarch.validator.Min";
      if(mErrorMessage)
      {
         detail["message"] = mErrorMessage;
      }
      detail["expectedMin"] = mSize;
   }
   else
   {
      context->addSuccess();
   }

   return rval;
}
