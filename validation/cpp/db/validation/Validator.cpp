/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/validation/Validator.h"

#include <cstdlib>

using namespace db::rt;
using namespace db::validation;

Validator::Validator(const char* errorMessage)
{
   mErrorMessage = errorMessage ? strdup(errorMessage) : NULL;
}

Validator::~Validator()
{
   if(mErrorMessage)
   {
      free(mErrorMessage);
   }
}

bool Validator::isValid(
      db::rt::DynamicObject& obj,
      ValidatorContext* context)
{
   context->addSuccess();
   return true;
}

bool Validator::isValid(db::rt::DynamicObject& obj)
{
   bool rval;
   ValidatorContext* ctx = new ValidatorContext();
   rval = isValid(obj, ctx);
   delete ctx;
   return rval;
}

bool Validator::isOptional(ValidatorContext* context)
{
   return false;
}
