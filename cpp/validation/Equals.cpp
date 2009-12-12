/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Equals.h"

using namespace monarch::rt;
using namespace monarch::validation;

Equals::Equals(monarch::rt::DynamicObject& object, const char* errorMessage) :
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
   monarch::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = (obj == mObject);
   if(!rval)
   {
      DynamicObject detail =
         context->addError("monarch.validation.EqualityFailure", &obj);
      detail["validator"] = "monarch.validator.Equals";
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
