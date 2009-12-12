/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Equals.h"

using namespace db::rt;
using namespace db::validation;

Equals::Equals(db::rt::DynamicObject& object, const char* errorMessage) :
   Validator(errorMessage),
   mObject(object)
{
}

Equals::Equals(const char* str, const char* errorMessage) :
   Validator(errorMessage)
{
   mObject = str;
}

Equals::Equals(bool b, const char* errorMessage) :
   Validator(errorMessage)
{
   mObject = b;
}

Equals::~Equals()
{
}

bool Equals::isValid(
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = (obj == mObject);
   if(!rval)
   {
      DynamicObject detail =
         context->addError("db.validation.EqualityFailure", &obj);
      detail["validator"] = "db.validator.Equals";
      detail["expectedValue"] = mObject;
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
