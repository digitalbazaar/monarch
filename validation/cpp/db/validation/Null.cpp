/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Null.h"

using namespace db::rt;
using namespace db::validation;

Null::Null(const char* errorMessage) :
   Validator(errorMessage)
{
}

Null::~Null()
{
}

bool Null::isValid(
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = obj.isNull();
   
   if(!rval)
   {
      DynamicObject detail =
         context->addError("db.validation.NotNullError", &obj);
      detail["validator"] = "db.validator.Null";
      detail["expectedValue"] = "null";
      if(mErrorMessage)
      {
         detail["message"] = mErrorMessage;
      }
      // FIXME: add expected value to error detail?
   }
   else
   {
      context->addSuccess();
   }
   
   return rval;
}
