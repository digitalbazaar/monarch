/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Optional.h"

using namespace monarch::rt;
using namespace monarch::validation;

Optional::Optional(Validator* validator) :
   mValidator(validator)
{
}

Optional::~Optional()
{
   delete mValidator;
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
