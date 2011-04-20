/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Optional.h"

using namespace monarch::rt;
using namespace monarch::validation;

Optional::Optional(Validator* validator) :
   mValidator(validator),
   mValidatorRef(NULL)
{
}

Optional::Optional(ValidatorRef& validator) :
   mValidator(&(*validator)),
   mValidatorRef(validator)
{
}

Optional::~Optional()
{
   if(mValidatorRef.isNull())
   {
      delete mValidator;
   }
}

bool Optional::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   return mValidator->isValid(obj, context);
}

bool Optional::isOptional(ValidatorContext* context)
{
   return true;
}
