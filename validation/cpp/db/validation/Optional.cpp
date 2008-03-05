/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Optional.h"

using namespace db::rt;
using namespace db::validation;

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
