/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/NotValid.h"

using namespace db::rt;
using namespace db::validation;

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
   DynamicObject detail = context->addError("db.validation.NotValid");
   if(mErrorMessage)
   {
      detail["message"] = mErrorMessage;
   }
   return false;
}
