/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
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
   bool rval = obj->length() >= mSize;
   
   if(!rval)
   {
      DynamicObject detail = context->addError("db.validation.MinError");
      if(mErrorMessage)
      {
         detail["message"] = mErrorMessage;
      }
      detail["expectedMin"] = mSize;
   }
   
   return rval;
}
