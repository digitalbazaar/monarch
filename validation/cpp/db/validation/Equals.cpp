/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Equals.h"

using namespace db::rt;
using namespace db::validation;

Equals::Equals(db::rt::DynamicObject& object, const char* errorMessage) :
   Validator(errorMessage),
   mObject(object)
{
}

Equals::~Equals()
{
}

bool Equals::isValid(
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = obj == mObject;
   if(!rval)
   {
      DynamicObject detail = context->addError("db.validation.EqualityFailure");
      if(mErrorMessage)
      {
         detail["message"] = mErrorMessage;
      }
      // FIXME: add expected value to error detail?
   }
   return rval;
}
