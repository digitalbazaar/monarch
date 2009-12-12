/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Validator.h"

#include <cstdlib>

using namespace monarch::rt;
using namespace monarch::validation;

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
      monarch::rt::DynamicObject& obj,
      ValidatorContext* context)
{
   context->addSuccess();
   return true;
}

bool Validator::isValid(monarch::rt::DynamicObject& obj)
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

size_t Validator::length()
{
   return 1;
}
