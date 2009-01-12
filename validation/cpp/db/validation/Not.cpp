/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Not.h"

using namespace db::rt;
using namespace db::validation;

Not::Not(Validator* validator, const char* errorMessage) :
   Validator(errorMessage),
   mValidator(validator)
{
}

Not::~Not()
{
   delete mValidator;
}

bool Not::isValid(
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   // save value
   bool setExceptions = context->setExceptions(false);
   // check sub-validator
   bool rval = !mValidator->isValid(obj, context);
   // restore
   context->setExceptions(setExceptions);
   
   if(!rval)
   {
      DynamicObject detail =
         context->addError("db.validation.ValueError", &obj);
      detail["validator"] = "db.validator.Not";
      if(mErrorMessage)
      {
         detail["message"] = mErrorMessage;
      }
   }
   else
   {
      context->addSuccess();
   }
   
   return rval;
}
