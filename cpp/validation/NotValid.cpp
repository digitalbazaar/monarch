/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/NotValid.h"

using namespace monarch::rt;
using namespace monarch::validation;

NotValid::NotValid(const char* errorMessage) :
   Validator(errorMessage)
{
}

NotValid::~NotValid()
{
}

bool NotValid::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   DynamicObject detail = context->addError("monarch.validation.NotValid", &obj);
   detail["validator"] = "monarch.validator.NotValid";
   if(mErrorMessage)
   {
      detail["message"] = mErrorMessage;
   }
   return false;
}
