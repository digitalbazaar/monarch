/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/validation/Min.h"

using namespace db::rt;
using namespace db::validation;

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
      DynamicObject detail = context->addError("db.validation.MinError", &obj);
      detail["validator"] = "db.validator.Min";
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