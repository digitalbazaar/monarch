/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Max.h"

using namespace monarch::rt;
using namespace monarch::validation;

Max::Max(int size, const char* errorMessage) :
   Validator(errorMessage),
   mSize(size)
{
}

Max::~Max()
{
}

bool Max::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = (!obj.isNull() && obj->length() <= mSize);

   if(!rval)
   {
      DynamicObject detail = context->addError("monarch.validation.MaxError", &obj);
      detail["validator"] = "monarch.validator.Max";
      if(mErrorMessage)
      {
         detail["message"] = mErrorMessage;
      }
      detail["expectedMax"] = mSize;
   }
   else
   {
      context->addSuccess();
   }

   return rval;
}
