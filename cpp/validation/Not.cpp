/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Not.h"

using namespace monarch::rt;
using namespace monarch::validation;

Not::Not(Validator* validator, const char* errorMessage) :
   Validator(errorMessage),
   mValidator(validator),
   mValidatorRef(NULL)
{
}

Not::Not(ValidatorRef& validator, const char* errorMessage) :
   Validator(errorMessage),
   mValidator(&(*validator)),
   mValidatorRef(validator)
{
}

Not::~Not()
{
   if(mValidatorRef.isNull())
   {
      delete mValidator;
   }
}

bool Not::isValid(
   monarch::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   // save state, ignore sub-validator results
   bool setExceptions = context->setExceptions(false);
   context->pushResults();
   // check sub-validator
   bool rval = !mValidator->isValid(obj, context);
   // restore state and results
   context->setExceptions(setExceptions);
   context->popResults();

   if(!rval)
   {
      DynamicObject detail =
         context->addError("monarch.validation.ValueError", &obj);
      detail["validator"] = "monarch.validator.Not";
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
